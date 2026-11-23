#include <smacc2/smacc.hpp>
#include <rclcpp/rclcpp.hpp>
#include <string>

namespace sc = boost::statechart;
namespace mpl = boost::mpl;

// Події переходів між станами
struct EvPillowsOk : sc::event<EvPillowsOk> {};
struct EvStainRetry : sc::event<EvStainRetry> {};
struct EvStainCleaned : sc::event<EvStainCleaned> {};
struct EvStainFailed : sc::event<EvStainFailed> {};
struct EvLinensReplaced : sc::event<EvLinensReplaced> {};
struct EvBlanketPlaced : sc::event<EvBlanketPlaced> {};

// Попереднє оголошення станів
struct StCheckBed;
struct StCleanStains;
struct StReplaceLinens;
struct StSelectBlanket;
struct StFinished;

// Головний клас скінченного автомата SMACC2
struct SmHotelCleaner : public smacc2::SmaccStateMachineBase<SmHotelCleaner, StCheckBed> {
  using SmaccStateMachineBase::SmaccStateMachineBase;

  int clean_attempts = 0;
  std::string season = "winter";

  void onInitialize() override {
    this->getNode()->declare_parameter<std::string>("season", "winter");
    season = this->getNode()->get_parameter("season").as_string();
    RCLCPP_INFO(getLogger(), "[SMACC2] Автомат запущено. Поточний сезон: %s", season.c_str());
  }
};

// Стан 1: Перевірка подушок та простирадл
struct StCheckBed : smacc2::SmaccState<StCheckBed, SmHotelCleaner> {
  using SmaccState::SmaccState;
  typedef mpl::list<
    smacc2::Transition<EvPillowsOk, StCleanStains>
  > reactions;

  void onEntry() {
    RCLCPP_INFO(getLogger(), "[SMACC2 -> StCheckBed] Перевірка наявності подушок та простирадл... Усе на місці.");
    this->postEvent<EvPillowsOk>();
  }
};

// Стан 2: Очищення плям (до 3 спроб)
struct StCleanStains : smacc2::SmaccState<StCleanStains, SmHotelCleaner> {
  using SmaccState::SmaccState;
  typedef mpl::list<
    smacc2::Transition<EvStainRetry, StCleanStains>,
    smacc2::Transition<EvStainCleaned, StReplaceLinens>,
    smacc2::Transition<EvStainFailed, StFinished>
  > reactions;

  void onEntry() {
    auto & sm = this->context<SmHotelCleaner>();
    sm.clean_attempts++;
    RCLCPP_INFO(getLogger(), "[SMACC2 -> StCleanStains] Спроба очищення плям #%d із 3...", sm.clean_attempts);

    if (sm.clean_attempts < 3) {
      RCLCPP_WARN(getLogger(), "[SMACC2 -> StCleanStains] Пляма ще залишилась. Повторна спроба!");
      this->postEvent<EvStainRetry>();
    } else if (sm.clean_attempts == 3) {
      RCLCPP_INFO(getLogger(), "[SMACC2 -> StCleanStains] Пляму успішно видалено на спробі #3!");
      this->postEvent<EvStainCleaned>();
    } else {
      RCLCPP_ERROR(getLogger(), "[SMACC2 -> StCleanStains] Перевищено ліміт спроб!");
      this->postEvent<EvStainFailed>();
    }
  }
};

// Стан 3: Заміна постільної білизни
struct StReplaceLinens : smacc2::SmaccState<StReplaceLinens, SmHotelCleaner> {
  using SmaccState::SmaccState;
  typedef mpl::list<
    smacc2::Transition<EvLinensReplaced, StSelectBlanket>
  > reactions;

  void onEntry() {
    RCLCPP_INFO(getLogger(), "[SMACC2 -> StReplaceLinens] Заміна простирадл та наволочок виконана.");
    this->postEvent<EvLinensReplaced>();
  }
};

// Стан 4: Вибір ковдри залежно від сезону
struct StSelectBlanket : smacc2::SmaccState<StSelectBlanket, SmHotelCleaner> {
  using SmaccState::SmaccState;
  typedef mpl::list<
    smacc2::Transition<EvBlanketPlaced, StFinished>
  > reactions;

  void onEntry() {
    const auto & season = this->context<SmHotelCleaner>().season;
    if (season == "winter") {
      RCLCPP_INFO(getLogger(), "[SMACC2 -> StSelectBlanket] Сезон 'winter': застеляємо теплу зимову ковдру.");
    } else {
      RCLCPP_INFO(getLogger(), "[SMACC2 -> StSelectBlanket] Сезон '%s': застеляємо легке літнє покривало.", season.c_str());
    }
    this->postEvent<EvBlanketPlaced>();
  }
};

// Стан 5: Завершення роботи
struct StFinished : smacc2::SmaccState<StFinished, SmHotelCleaner> {
  using SmaccState::SmaccState;

  void onEntry() {
    RCLCPP_INFO(getLogger(), "[SMACC2 -> StFinished] Прибирання номера повністю завершено! (Натисніть Ctrl+C для виходу)");
  }
};

int main(int argc, char ** argv) {
  rclcpp::init(argc, argv);
  smacc2::run<SmHotelCleaner>();
  rclcpp::shutdown();
  return 0;
}
