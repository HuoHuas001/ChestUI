#include "pch.h"
#include <llapi/EventAPI.h>
#include <llapi/LoggerAPI.h>
#include <llapi/mc/Level.hpp>
#include <llapi/mc/Actor.hpp>
#include <llapi/mc/Player.hpp>
#include "Version.h"
#include <llapi/LLAPI.h>
#include <llapi/HookAPI.h>

Logger logger(PLUGIN_NAME);

// #include <mc/LevelChunk.hpp>
// #include <mc/Dimension.hpp>
// #include <mc/ChunkSource.hpp>
// #include <mc/DBChunkStorage.hpp>
// #include <mc/ChunkPos.hpp>
//
// THook(void, "?loadChunk@DBChunkStorage@@UEAAXAEAVLevelChunk@@_N@Z", DBChunkStorage* _this, LevelChunk* a2, bool a3) {
//     auto& pos = a2->getPosition();
//     logger.warn("DBChunkStorage::loadChunk : LevelChunkPos: {}, {}; bool: {}", pos.x, pos.z, a3);
//     return original(_this, a2, a3);
// }
//
// THook(__int64, "?createNewChunk@ChunkSource@@UEAA?AV?$shared_ptr@VLevelChunk@@@std@@AEBVChunkPos@@W4LoadMode@1@_N@Z", __int64 a1, __int64* a2, __int64* a3, int a4, unsigned __int8 a5) {
//     logger.warn("ChunkSource::createNewChunk : ChunkPos: {}, {}; LoadMode: {}; bool: {}", *a2, *a3, a4, a5);
//     return original(a1, a2, a3, a4, a5);
// }
//
// THook(UINT64, "?getOrLoadChunk@ChunkSource@@UEAA?AV?$shared_ptr@VLevelChunk@@@std@@AEBVChunkPos@@W4LoadMode@1@_N@Z", ChunkSource* _this,
//     UINT64* a2,
//     ChunkPos* a3,
//     unsigned int a4,
//     char a5) {
//     logger.warn("ChunkSource::getOrLoadChunk : ChunkPos: {}, {}; LoadMode: {}; bool: {}", a3->x, a3->z, a4, a5);
//     return original(_this, a2, a3, a4, a5);
// }

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
#include <llapi/RegCommandAPI.h>

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
    RecordedInfo[pl] = pos;
}

void updateBlockActor(Player *pl, const BlockPos &pos, const string &title, const std::unordered_map<int, string> &option)
{
    string chest = "{\"CustomName\":\"\",\"Findable\":0b,\"Items\":[";
    if (!option.empty())
    {
        chest += addSoltTag(option.begin()->second, option.begin()->first);
        for (auto &it = ++option.begin(); it != option.end(); ++it)
        {
            chest += ("," + addSoltTag(it->second, it->first));
        }
    }
    chest += "],\"id\":\"Chest\",\"isMovable\":1b,\"x\":0,\"y\":0,\"z\":0}";

    auto nbt = CompoundTag::fromSNBT(chest);

    nbt->putInt("x", pos.x);
    nbt->putInt("y", pos.y);
    nbt->putInt("z", pos.z);
    nbt->putString("CustomName", title);

    BinaryStream bs;
    bs.writeVarInt(pos.x);
    bs.writeUnsignedVarInt(pos.y);
    bs.writeVarInt(pos.z);
    bs.writeCompoundTag(*nbt);
    auto pkt = MinecraftPackets::createPacket(MinecraftPacketIds::BlockActorData);
    pkt->read(bs);
    pl->sendNetworkPacket(*pkt);
}

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

void OpenContainerUI(Player *pl, string title, std::unordered_map<int, string> option)
{
    auto pos = pl->getBlockPos();
    updateBlock(pl, pos);
    updateBlockActor(pl, pos, title, option);
    open(pl, pos);
}

class openCommand : public Command
{
public:
    void execute(CommandOrigin const &ori, CommandOutput &output) const override
    { // 执行部分
        ServerPlayer *sp = ori.getPlayer();
        std::unordered_map<int, string> v = {
            {0, "{\"Block\":{\"name\":\"minecraft:enchanting_table\",\"states\":{},\"version\":18087969},\"Count\":64b,\"Damage\":0s,\"Name\":\"minecraft:enchanting_table\",\"WasPickedUp\":0b,\"tag\":{\"display\":{\"Lore\":[\"测试选项1\",\"测试2\",\"测3\"]}}}"},
            {1, "{\"Block\":{\"name\":\"minecraft:enchanting_table\",\"states\":{},\"version\":18087969},\"Count\":64b,\"Damage\":0s,\"Name\":\"minecraft:enchanting_table\",\"WasPickedUp\":0b,\"tag\":{\"display\":{\"Lore\":[\"测1\",\"测2\",\"测3\"]}}}"},
            {2, "{\"Block\":{\"name\":\"minecraft:enchanting_table\",\"states\":{},\"version\":18087969},\"Count\":64b,\"Damage\":0s,\"Name\":\"minecraft:enchanting_table\",\"WasPickedUp\":0b,\"tag\":{\"display\":{\"Lore\":[\"114514\",\"1919810\"]}}}"}};
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

    Event::ConsoleCmdEvent::subscribe([](const Event::ConsoleCmdEvent &ev)
                                      { return true; });
}

THook(void, "?handle@ServerNetworkHandler@@UEAAXAEBVNetworkIdentifier@@AEBVContainerClosePacket@@@Z",
      ServerNetworkHandler *_this, NetworkIdentifier *a2, ContainerClosePacket *a3)
{
    Player *pl = (Player *)_this->getServerPlayer(*a2, 0);
    auto it = RecordedInfo.find(pl);
    if (it != RecordedInfo.end())
    {
        pl->sendUpdateBlockPacket(it->second, pl->getBlockSource()->getBlock(it->second).getRuntimeId());
        RecordedInfo.erase(it);
    }

    return original(_this, a2, a3);
}
