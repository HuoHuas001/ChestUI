#pragma once
#include <llapi/mc/Level.hpp>
#include <llapi/mc/Actor.hpp>
#include <llapi/mc/Player.hpp>
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

using std::string;

struct ContainerUIData
{
    Player *pl;
    std::unordered_map<int, string> option;
    BlockPos bp;
    void (*func)(Player *, int);
    bool bigChest;
    bool forceClose;
    bool isSwitch;
};

extern std::unordered_map<Player *, ContainerUIData> RecordedInfo;

namespace chestUI{
    string addSoltTag(const string &str, int solt);
    void updateBlock(Player *pl, const BlockPos &pos,bool bigChest);
    void updateBlockActor(
        Player *pl, 
        const BlockPos &pos, 
        const string &title, 
        const std::unordered_map<int, string> &option,
        bool bigChest = false);
    void open(Player *pl, const BlockPos &pos);
    void close(Player *pl, const BlockPos &pos,bool isSwitch = false);
    void OpenContainerUI(
        Player *pl, 
        string title, 
        std::unordered_map<int, string> option,
        void (*func)(Player *, int),
        bool bigChest = false
        );
    void CloseContainerUI(Player *pl);
    void callback(Player*pl,int slot);
}