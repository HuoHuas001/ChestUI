#include <llapi/EventAPI.h>
#include <llapi/LoggerAPI.h>
#include "Version.h"
#include "ChestUI.hpp"

#define _QWORD unsigned __int64
Logger logger(PLUGIN_NAME);
std::unordered_map<Player *, ContainerUIData> RecordedInfo;

void UICallBack(Player *pl, int slot)
{
    if (slot < 0)
    {
        pl->sendText("你刚刚关闭了箱子 ");
    }

    if (slot == 13)
    {
        auto it = RecordedInfo[pl];
        chestUI::close(pl, it.bp, true);
        pl->sendText("你刚刚选择了个人信息 ");
        Global<Level>->runcmd("kill @e");
        std::unordered_map<int, string> v = {
            {13, "{\"Count\":1b,\"Damage\":3s,\"Name\":\"minecraft:skull\",\"Slot\":13b,\"WasPickedUp\":0b,\"tag\":{\"RepairCost\":0,\"display\":{\"Name\":\"个人信息\"}}}"},
            {19, "{\"Count\":1b,\"Damage\":0s,\"Name\":\"minecraft:diamond\",\"Slot\":19b,\"WasPickedUp\":0b,\"tag\":{\"RepairCost\":0,\"display\":{\"Name\":\"商店\"}}}"},
            {21, "{\"Count\":1b,\"Damage\":0s,\"Name\":\"minecraft:wooden_door\",\"Slot\":21b,\"WasPickedUp\":0b,\"tag\":{\"RepairCost\":0,\"display\":{\"Name\":\"我的家园\"}}}"},
            {23, "{\"Block\":{\"name\":\"minecraft:stained_glass\",\"states\":{\"color\":\"gray\"},\"version\":18087969},\"Count\":1b,\"Damage\":0s,\"Name\":\"minecraft:stained_glass\",\"Slot\":23b,\"WasPickedUp\":0b,\"tag\":{\"RepairCost\":0,\"display\":{\"Name\":\"工会菜单\"}}}"},
            {25, "{\"Block\":{\"name\":\"minecraft:chest\",\"states\":{\"facing_direction\":0},\"version\":18087969},\"Count\":1b,\"Damage\":0s,\"Name\":\"minecraft:chest\",\"Slot\":25b,\"WasPickedUp\":0b,\"tag\":{\"RepairCost\":0,\"display\":{\"Name\":\"我的背包\"}}}"},
        };
        chestUI::updateBlockActor(pl, it.bp, "Chest Test 2", v);
        Schedule::delay([pl, it]()
                        { chestUI::open(pl, it.bp); },
                        0.3);
    }
};

// 注册命令
class openCommand : public Command
{
public:
    void execute(CommandOrigin const &ori, CommandOutput &output) const override
    { // 执行部分
        ServerPlayer *sp = ori.getPlayer();
        // 放入第一个箱子即为Slot显示的数字，第二个箱子即为26+Slot显示的数字(可以无需去掉Slot，会自己覆盖)
        std::unordered_map<int, string> v = {
            {13, "{\"Count\":1b,\"Damage\":3s,\"Name\":\"minecraft:skull\",\"Slot\":13b,\"WasPickedUp\":0b,\"tag\":{\"RepairCost\":0,\"display\":{\"Name\":\"个人信息\"}}}"},
            {19, "{\"Count\":1b,\"Damage\":0s,\"Name\":\"minecraft:diamond\",\"Slot\":19b,\"WasPickedUp\":0b,\"tag\":{\"RepairCost\":0,\"display\":{\"Name\":\"商店\"}}}"},
            {21, "{\"Count\":1b,\"Damage\":0s,\"Name\":\"minecraft:wooden_door\",\"Slot\":21b,\"WasPickedUp\":0b,\"tag\":{\"RepairCost\":0,\"display\":{\"Name\":\"我的家园\"}}}"},
            {23, "{\"Block\":{\"name\":\"minecraft:stained_glass\",\"states\":{\"color\":\"gray\"},\"version\":18087969},\"Count\":1b,\"Damage\":0s,\"Name\":\"minecraft:stained_glass\",\"Slot\":23b,\"WasPickedUp\":0b,\"tag\":{\"RepairCost\":0,\"display\":{\"Name\":\"工会菜单\"}}}"},
            {25, "{\"Block\":{\"name\":\"minecraft:chest\",\"states\":{\"facing_direction\":0},\"version\":18087969},\"Count\":1b,\"Damage\":0s,\"Name\":\"minecraft:chest\",\"Slot\":25b,\"WasPickedUp\":0b,\"tag\":{\"RepairCost\":0,\"display\":{\"Name\":\"我的背包\"}}}"},
            {37, "{\"Block\":{\"name\":\"minecraft:command_block\",\"states\":{\"conditional_bit\":0b,\"facing_direction\":0},\"version\":18087969},\"Count\":1b,\"Damage\":0s,\"Name\":\"minecraft:command_block\",\"Slot\":11b,\"WasPickedUp\":0b,\"tag\":{\"RepairCost\":0,\"display\":{\"Name\":\"管理菜单\"}}}"},
            {39, "{\"Block\":{\"name\":\"minecraft:redstone_lamp\",\"states\":{},\"version\":18087969},\"Count\":1b,\"Damage\":0s,\"Name\":\"minecraft:redstone_lamp\",\"Slot\":13b,\"WasPickedUp\":0b,\"tag\":{\"RepairCost\":0,\"display\":{\"Name\":\"开关服务器\"}}}"},
            {41, "{\"Block\":{\"name\":\"minecraft:enchanting_table\",\"states\":{},\"version\":18087969},\"Count\":1b,\"Damage\":0s,\"Name\":\"minecraft:enchanting_table\",\"WasPickedUp\":0b,\"tag\":{\"display\":{\"Lore\":[\"NMSL\"]}}}"},
        };
        chestUI::OpenContainerUI(sp, "Chest Form Test", v, UICallBack);
        //logger.info("runtimeId {}",StaticVanillaBlocks::mChest->getRuntimeId());
    }

    static void setup(CommandRegistry *registry)
    { // 注册部分(推荐做法)
        registry->registerCommand("open", "open chest ui", CommandPermissionLevel::Any, {(CommandFlagValue)0}, {(CommandFlagValue)0x80});
        registry->registerOverload<openCommand>("open");
    }
};

void PluginInit()
{
    Event::RegCmdEvent::subscribe([](const Event::RegCmdEvent &ev)
                                  {
    openCommand::setup(ev.mCommandRegistry);
    return true; });
}

// 关闭容器刷新方块
THook(void, "?handle@ServerNetworkHandler@@UEAAXAEBVNetworkIdentifier@@AEBVContainerClosePacket@@@Z",
      ServerNetworkHandler *_this, NetworkIdentifier *a2, ContainerClosePacket *a3)
{
    Player *pl = (Player *)_this->getServerPlayer(*a2, 0);
    auto it = RecordedInfo.find(pl);
    // 回调，请自行完善功能
    if (it != RecordedInfo.end())
    {
        if (!it->second.isSwitch)
        {
            if (!it->second.forceClose)
            {
                chestUI::callback(pl, -1);
            }
            chestUI::CloseContainerUI(pl);
        }
        else
        {
            it->second.isSwitch = false;
        }
    }

    return original(_this, a2, a3);
}

// Player选择回调
THook(char, "?_handleTransfer@ItemStackRequestActionHandler@@AEAA?AW4ItemStackNetResult@@AEBVItemStackRequestActionTransferBase@@_N11@Z",
      ItemStackRequestActionHandler *_this,
      ItemStackRequestActionTransferBase *a2,
      bool a3,
      bool a4,
      bool a5)
{
    Player *pl = ((Player *)(*(_QWORD *)_this));
    auto it = RecordedInfo.find(pl);
    // 回调，请自行完善功能
    if (it != RecordedInfo.end())
    {
        ItemStackRequestSlotInfo *modifier = const_cast<ItemStackRequestSlotInfo *>(&a2->getSrc());
        int slot = *(((UINT8 *)modifier) + 1); // 玩家选择的Slot
        chestUI::callback(pl, slot);
    }

    return original(_this, a2, a3, a4, a5);
}