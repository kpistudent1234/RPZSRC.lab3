#include <algorithm>
#include <cmath>
#include <deque>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "rclcpp/rclcpp.hpp"

struct SimPoint {
  double t;
  double x;
  double v;
  double u;
};

class PRegulatorSimulator : public rclcpp::Node {
public:
  PRegulatorSimulator() : Node("p_regulator_sim") {}

  std::vector<SimPoint> run_simulation(
    double Kp,
    double T,
    double a_max,
    double x_target,
    double dt_actuation,
    double dt_feedback,
    bool use_compensation,
    double duration = 6.0)
  {
    std::vector<SimPoint> history;

    // Кількість кроків затримки у дискретному часі
    size_t act_steps = static_cast<size_t>(std::round(dt_actuation / T));
    size_t fb_steps = static_cast<size_t>(std::round(dt_feedback / T));
    double dt_loop = dt_actuation + dt_feedback;

    // Буфери FIFO для моделювання затримок лінії зв'язку та приводу
    std::deque<double> act_buffer(act_steps, 0.0);
    std::deque<std::pair<double, double>> fb_buffer(fb_steps, {0.0, 0.0});

    double x = 0.0;
    double v = 0.0;
    int total_steps = static_cast<int>(std::round(duration / T));

    for (int step = 0; step <= total_steps; ++step) {
      double t = step * T;

      // 1. Зчитування даних із енкодера (з урахуванням затримки зворотного зв'язку)
      double x_meas = x;
      double v_meas = v;
      if (fb_steps > 0) {
        fb_buffer.push_back({x, v});
        x_meas = fb_buffer.front().first;
        v_meas = fb_buffer.front().second;
        fb_buffer.pop_front();
      }

      // 2. Розрахунок помилки (звичайний або з лінійним предиктором)
      double error = 0.0;
      if (use_compensation) {
        double x_pred = x_meas + v_meas * dt_loop;
        error = x_target - x_pred;
      } else {
        error = x_target - x_meas;
      }

      // 3. Обчислення керуючого впливу П-регулятора
      double u_cmd = Kp * error;

      // 4. Затримка актуації (виконавчого механізму)
      double u_applied = u_cmd;
      if (act_steps > 0) {
        act_buffer.push_back(u_cmd);
        u_applied = act_buffer.front();
        act_buffer.pop_front();
      }

      history.push_back({t, x, v, u_applied});

      // 5. Фізична модель суглоба з обмеженням прискорення/гальмування a_max
      double desired_accel = (u_applied - v) / T;
      double actual_accel = std::clamp(desired_accel, -a_max, a_max);

      x += v * T + 0.5 * actual_accel * T * T;
      v += actual_accel * T;
    }

    return history;
  }

  void export_all_scenarios() {
    // Базові параметри з методичних вказівок
    const double Kp = 5.0;
    const double T = 0.05;       // 50 мс
    const double a_max = 0.5;    // 0.5 рад/с^2
    const double x_target = 0.05; // 0.05 рад

    // Сценарій 1: Нульові затримки
    auto ideal = run_simulation(Kp, T, a_max, x_target, 0.0, 0.0, false);

    // Сценарій 2: Затримки dt_act = 100 мс, dt_fb = 50 мс (без компенсації)
    auto delayed = run_simulation(Kp, T, a_max, x_target, 0.10, 0.05, false);

    // Сценарій 3: Затримки 150 мс + лінійний предиктор компенсації
    auto compensated = run_simulation(Kp, T, a_max, x_target, 0.10, 0.05, true);

    std::ofstream file("/ros2_ws/sim_main.csv");
    file << "t,x_ideal,v_ideal,x_delayed,v_delayed,x_comp,v_comp\n";
    for (size_t i = 0; i < ideal.size(); ++i) {
      file << ideal[i].t << ","
           << ideal[i].x << "," << ideal[i].v << ","
           << delayed[i].x << "," << delayed[i].v << ","
           << compensated[i].x << "," << compensated[i].v << "\n";
    }
    file.close();

    // Додаткові сценарії для дослідження впливу Kp, T та dt_loop
    auto kp_high = run_simulation(8.0, T, a_max, x_target, 0.10, 0.05, false);
    auto kp_low = run_simulation(2.5, T, a_max, x_target, 0.10, 0.05, false);
    auto dt_large = run_simulation(Kp, T, a_max, x_target, 0.15, 0.10, false);

    std::ofstream sens_file("/ros2_ws/sim_sensitivity.csv");
    sens_file << "t,x_base_delay,x_kp_high,x_kp_low,x_dt_250ms\n";
    for (size_t i = 0; i < delayed.size(); ++i) {
      sens_file << delayed[i].t << ","
                << delayed[i].x << ","
                << kp_high[i].x << ","
                << kp_low[i].x << ","
                << dt_large[i].x << "\n";
    }
    sens_file.close();

    RCLCPP_INFO(this->get_logger(), "Моделювання завершено! Дані збережено у /ros2_ws/sim_main.csv та /ros2_ws/sim_sensitivity.csv");
  }
};

int main(int argc, char ** argv) {
  rclcpp::init(argc, argv);
  auto node = std::make_shared<PRegulatorSimulator>();
  node->export_all_scenarios();
  rclcpp::shutdown();
  return 0;
}
