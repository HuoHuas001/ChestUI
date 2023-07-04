#include <llapi/EventAPI.h>
#include <llapi/LoggerAPI.h>
#include <llapi/mc/Level.hpp>
#include <llapi/mc/Actor.hpp>
#include <llapi/mc/Player.hpp>
#include "Version.h"
#include <llapi/LLAPI.h>
#include <llapi/HookAPI.h>
#include <llapi/RegCommandAPI.h>
#include <llapi/mc/ItemStackRequestActionHandler.hpp>
#include <llapi/mc/ItemStackRequestActionTransferBase.hpp>
#include <llapi/mc/ItemStackRequestSlotInfo.hpp>
#include <llapi/mc/BlockInstance.hpp>
#include <llapi/mc/Block.hpp>
#include <llapi/mc/BlockActor.hpp>
#include <llapi/mc/BlockSource.hpp>
#include <llapi/mc/BlockActorDataPacket.hpp>
#include <llapi/mc/CompoundTag.hpp>
#include <llapi/mc/Vec3.hpp>
#include <llapi/mc/BlockPos.hpp>
#include <llapi/mc/BinaryStream.hpp>
#include <llapi/mc/ExtendedStreamReadResult.hpp>
#include <llapi/mc/MinecraftPackets.hpp>
#include <llapi/mc/Packet.hpp>
#include <llapi/SendPacketAPI.h>
#include <llapi/mc/StaticVanillaBlocks.hpp>
#include <llapi/mc/ServerNetworkHandler.hpp>
#include <llapi/mc/NetworkIdentifier.hpp>
#include <llapi/mc/ContainerClosePacket.hpp>
#include <llapi/ScheduleAPI.h>

#define _QWORD unsigned __int64
Logger logger(PLUGIN_NAME);

std::unordered_map<Player *, BlockPos> RecordedInfo;

string addSoltTag(const string &str, int solt)
{
    auto item = CompoundTag::fromSNBT(str);
    item->putByte("Slot", solt);
    return item->toSNBT();
}

void updateBlock(Player *pl, const BlockPos &pos)
{
    pl->sendUpdateBlockPacket(pos, StaticVanillaBlocks::mChest->getRuntimeId());
    pl->sendUpdateBlockPacket(pos.add(1, 0, 0), StaticVanillaBlocks::mChest->getRuntimeId());
    RecordedInfo[pl] = pos;
}

void updateBlockActor(Player *pl, const BlockPos &pos, const string &title, const std::unordered_map<int, string> &option)
{
    string chest1 = "{\"CustomName\":\"\",\"Findable\":0b,\"Items\":[";
    if (!option.empty())
    {
        chest1 += addSoltTag(option.begin()->second, option.begin()->first);
        for (auto &it = ++option.begin(); it != option.end(); ++it)
        {
            if (it->first <= 26)
            {
                chest1 += ("," + addSoltTag(it->second, it->first));
            }
        }
    }
    chest1 += "],\"id\":\"Chest\",\"isMovable\":1b,\"x\":0,\"y\":0,\"z\":0}";

    string chest2 = "{\"CustomName\":\"\",\"Findable\":0b,\"Items\":[";
    if (!option.empty())
    {
        bool first = true;
        for (auto &it = ++option.begin(); it != option.end(); ++it)
        {
            if (it->first > 26)
            {
                if (first)
                {
                    chest2 += addSoltTag(it->second, it->first - 26);
                    first = false;
                }
                else
                {
                    chest2 += ("," + addSoltTag(it->second, it->first - 26));
                }
            }
        }
    }
    chest2 += "],\"id\":\"Chest\",\"isMovable\":1b,\"x\":0,\"y\":0,\"z\":0}";

    auto nbt1 = CompoundTag::fromSNBT(chest1);
    nbt1->putInt("x", pos.x);
    nbt1->putInt("y", pos.y);
    nbt1->putInt("z", pos.z);
    nbt1->putInt("pairx", pos.x + 1);
    nbt1->putInt("pairz", pos.z);
    nbt1->putByte("pairlead", 1);
    nbt1->putString("CustomName", title);

    auto nbt2 = CompoundTag::fromSNBT(chest2);
    nbt2->putInt("x", pos.x + 1);
    nbt2->putInt("y", pos.y);
    nbt2->putInt("z", pos.z);
    nbt2->putInt("pairx", pos.x);
    nbt2->putInt("pairz", pos.z);
    nbt2->putByte("pairlead", 0);
    nbt2->putString("CustomName", title);

    // 放置箱子1
    BinaryStream bs1;
    bs1.writeVarInt(pos.x);
    bs1.writeUnsignedVarInt(pos.y);
    bs1.writeVarInt(pos.z);
    bs1.writeCompoundTag(*nbt1);
    auto pkt1 = MinecraftPackets::createPacket(MinecraftPacketIds::BlockActorData);
    pkt1->read(bs1);
    pl->sendNetworkPacket(*pkt1);
    // 放置箱子2
    BinaryStream bs2;
    bs2.writeVarInt(pos.x + 1);
    bs2.writeUnsignedVarInt(pos.y);
    bs2.writeVarInt(pos.z);
    bs2.writeCompoundTag(*nbt2);
    auto pkt2 = MinecraftPackets::createPacket(MinecraftPacketIds::BlockActorData);
    pkt2->read(bs2);
    pl->sendNetworkPacket(*pkt2);
}

// 打开容器包
void open(Player *pl, const BlockPos &pos)
{
    BinaryStream bs;
    bs.writeByte(0);
    bs.writeByte(0);
    bs.writeVarInt(pos.x);
    bs.writeUnsignedVarInt(pos.y);
    bs.writeVarInt(pos.z);
    bs.writeVarInt64(-1);
    auto pkt = MinecraftPackets::createPacket(MinecraftPacketIds::ContainerOpen);
    pkt->read(bs);
    pl->sendNetworkPacket(*pkt);
}

// 打开箱子UI
void OpenContainerUI(Player *pl, string title, std::unordered_map<int, string> option)
{
    // if(pl->getPlatform())
    auto pos = pl->getBlockPos();
    pos = pos.add(0, 5, 0);
    updateBlock(pl, pos);
    updateBlockActor(pl, pos, title, option);
    Schedule::delay([pl, pos]
                    { open(pl, pos); },
                    0.1);
}

// 关闭箱子UI
void CloseContainerUI(Player *pl)
{
    auto it = RecordedInfo.find(pl);
    if (it != RecordedInfo.end())
    {
        pl->sendUpdateBlockPacket(it->second, pl->getBlockSource()->getBlock(it->second).getRuntimeId());
        pl->sendUpdateBlockPacket(it->second.add(1, 0, 0), pl->getBlockSource()->getBlock(it->second).getRuntimeId());
        RecordedInfo.erase(it);
    }
}

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
        OpenContainerUI(sp, "Chest Form Test", v);
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
    CloseContainerUI(pl);
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
    //回调，请自行完善功能
    if (it != RecordedInfo.end())
    {
        ItemStackRequestSlotInfo *modifier = const_cast<ItemStackRequestSlotInfo *>(&a2->getSrc());
        int slot = *(((UINT8 *)modifier) + 1); //玩家选择的Slot
        logger.info("{} Choose {}", pl->getName(), slot);
    }

    return original(_this, a2, a3, a4, a5);
}