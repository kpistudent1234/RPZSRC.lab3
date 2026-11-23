# Лабораторна робота №3: Організація задач робота та інтеграція цифрових контролерів у ROS 2

## Структура проєкту
* `Dockerfile`, `entrypoint.sh` — конфігурація контейнера ROS 2 Jazzy з пакетами `behaviortree_cpp`, `smacc2`, `ros2_control` та `matplotlib`.
* `ros2_ws/src/hotel_cleaner_pkg` — реалізація логіки сервісного робота прибирання номерів на базі скінченного автомата (`SMACC2`) та дерева поведінки (`BehaviorTree.CPP`).
* `ros2_ws/src/joint_control_sim` — моделювання пропорційного регулятора приводу суглоба з часовими затримками ($\Delta t_{actuation} = 100\text{ мс}$, $\Delta t_{feedback} = 50\text{ мс}$) та лінійним предиктором компенсації.
* `ros2_ws/test_robot.urdf`, `ros2_ws/controllers.yaml` — конфігурація віртуального приводу та контролерів для `ros2_control`.
* `ros2_ws/plot_graphs.py`, `*.csv`, `*.png` — скрипти аналізу та графіки перехідних процесів.
