#include <iostream>
#include <string>
#include "rclcpp/rclcpp.hpp"
#include "behaviortree_cpp/bt_factory.h"

// 1. Вузол перевірки подушок та простирадл
class CheckPillowsAndSheets : public BT::SyncActionNode {
public:
  CheckPillowsAndSheets(const std::string & name, const BT::NodeConfig & config)
  : BT::SyncActionNode(name, config) {}

  static BT::PortsList providedPorts() { return {}; }

  BT::NodeStatus tick() override {
    std::cout << "[BT -> CheckPillowsAndSheets] Перевірка наявності подушок та простирадл: OK (SUCCESS)\n";
    return BT::NodeStatus::SUCCESS;
  }
};

// 2. Вузол очищення плям (імітує успіх на 3-й спробі)
class CleanStainAttempt : public BT::SyncActionNode {
public:
  CleanStainAttempt(const std::string & name, const BT::NodeConfig & config)
  : BT::SyncActionNode(name, config) {}

  static BT::PortsList providedPorts() { return {}; }

  BT::NodeStatus tick() override {
    attempt_++;
    std::cout << "[BT -> CleanStainAttempt] Спроба очищення плям #" << attempt_ << "... ";
    if (attempt_ < 3) {
      std::cout << "Пляма ще є (FAILURE)\n";
      return BT::NodeStatus::FAILURE;
    }
    std::cout << "Пляму видалено! (SUCCESS)\n";
    return BT::NodeStatus::SUCCESS;
  }

private:
  int attempt_ = 0;
};

// 3. Вузол заміни постільної білизни
class ReplaceLinens : public BT::SyncActionNode {
public:
  ReplaceLinens(const std::string & name, const BT::NodeConfig & config)
  : BT::SyncActionNode(name, config) {}

  static BT::PortsList providedPorts() { return {}; }

  BT::NodeStatus tick() override {
    std::cout << "[BT -> ReplaceLinens] Постільну білизну замінено на свіжу (SUCCESS)\n";
    return BT::NodeStatus::SUCCESS;
  }
};

// 4. Вузол-умова перевірки сезону з Blackboard
class IsWinterSeason : public BT::ConditionNode {
public:
  IsWinterSeason(const std::string & name, const BT::NodeConfig & config)
  : BT::ConditionNode(name, config) {}

  static BT::PortsList providedPorts() {
    return { BT::InputPort<std::string>("season") };
  }

  BT::NodeStatus tick() override {
    auto res = getInput<std::string>("season");
    if (!res) {
      throw BT::RuntimeError("Відсутній порт [season]: ", res.error());
    }
    bool is_winter = (res.value() == "winter");
    std::cout << "[BT -> IsWinterSeason] Перевірка сезону ('" << res.value() << "'): "
              << (is_winter ? "TRUE (SUCCESS)" : "FALSE (FAILURE)") << "\n";
    return is_winter ? BT::NodeStatus::SUCCESS : BT::NodeStatus::FAILURE;
  }
};

// 5. Дії вибору ковдри
class PutWinterBlanket : public BT::SyncActionNode {
public:
  PutWinterBlanket(const std::string & name, const BT::NodeConfig & config)
  : BT::SyncActionNode(name, config) {}
  static BT::PortsList providedPorts() { return {}; }
  BT::NodeStatus tick() override {
    std::cout << "[BT -> PutWinterBlanket] Застелено теплу зимову ковдру (SUCCESS)\n";
    return BT::NodeStatus::SUCCESS;
  }
};

class PutSummerBlanket : public BT::SyncActionNode {
public:
  PutSummerBlanket(const std::string & name, const BT::NodeConfig & config)
  : BT::SyncActionNode(name, config) {}
  static BT::PortsList providedPorts() { return {}; }
  BT::NodeStatus tick() override {
    std::cout << "[BT -> PutSummerBlanket] Застелено легку літню ковдру (SUCCESS)\n";
    return BT::NodeStatus::SUCCESS;
  }
};

// Опис структури дерева у форматі XML
static const char * xml_tree = R"(
<root BTCPP_format="4" main_tree_to_execute="HotelCleaningTree">
  <BehaviorTree ID="HotelCleaningTree">
    <Sequence name="MainCleaningSequence">
      <CheckPillowsAndSheets/>
      <RetryUntilSuccessful num_attempts="3">
        <CleanStainAttempt/>
      </RetryUntilSuccessful>
      <ReplaceLinens/>
      <Fallback name="SeasonalBlanketSelector">
        <Sequence name="WinterRoutine">
          <IsWinterSeason season="{season}"/>
          <PutWinterBlanket/>
        </Sequence>
        <PutSummerBlanket/>
      </Fallback>
    </Sequence>
  </BehaviorTree>
</root>
)";

int main(int argc, char ** argv) {
  rclcpp::init(argc, argv);
  auto node = std::make_shared<rclcpp::Node>("bt_cleaner_node");
  node->declare_parameter<std::string>("season", "winter");
  std::string current_season = node->get_parameter("season").as_string();

  std::cout << "\n=== Запуск BehaviorTree.CPP (Сезон: " << current_season << ") ===\n";

  BT::BehaviorTreeFactory factory;
  factory.registerNodeType<CheckPillowsAndSheets>("CheckPillowsAndSheets");
  factory.registerNodeType<CleanStainAttempt>("CleanStainAttempt");
  factory.registerNodeType<ReplaceLinens>("ReplaceLinens");
  factory.registerNodeType<IsWinterSeason>("IsWinterSeason");
  factory.registerNodeType<PutWinterBlanket>("PutWinterBlanket");
  factory.registerNodeType<PutSummerBlanket>("PutSummerBlanket");

  auto blackboard = BT::Blackboard::create();
  blackboard->set("season", current_season);

  auto tree = factory.createTreeFromText(xml_tree, blackboard);
  BT::NodeStatus status = tree.tickWhileRunning();

  std::cout << "=== Роботу дерева завершено зі статусом: " << BT::toStr(status) << " ===\n\n";
  rclcpp::shutdown();
  return 0;
}
