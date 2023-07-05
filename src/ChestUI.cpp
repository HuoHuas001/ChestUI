#include "ChestUI.hpp"

#define _QWORD unsigned __int64
using namespace chestUI;

string chestUI::addSoltTag(const string &str, int solt)
{
    auto item = CompoundTag::fromSNBT(str);
    item->putByte("Slot", solt);
    return item->toSNBT();
}

void chestUI::updateBlock(Player *pl, const BlockPos &pos, bool bigChest)
{
    pl->sendUpdateBlockPacket(pos, StaticVanillaBlocks::mChest->getRuntimeId());
    if (bigChest)
    {
        pl->sendUpdateBlockPacket(pos.add(1, 0, 0), StaticVanillaBlocks::mChest->getRuntimeId());
    }
}

void chestUI::updateBlockActor(
    Player *pl,
    const BlockPos &pos,
    const string &title,
    const std::unordered_map<int, string> &option,
    bool bigChest)
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

    auto nbt1 = CompoundTag::fromSNBT(chest1);
    nbt1->putInt("x", pos.x);
    nbt1->putInt("y", pos.y);
    nbt1->putInt("z", pos.z);
    nbt1->putInt("pairx", pos.x + 1);
    nbt1->putInt("pairz", pos.z);
    nbt1->putByte("pairlead", 1);
    nbt1->putString("CustomName", title);

    // 放置箱子1
    BinaryStream bs1;
    bs1.writeVarInt(pos.x);
    bs1.writeUnsignedVarInt(pos.y);
    bs1.writeVarInt(pos.z);
    bs1.writeCompoundTag(*nbt1);
    auto pkt1 = MinecraftPackets::createPacket(MinecraftPacketIds::BlockActorData);
    pkt1->read(bs1);
    pl->sendNetworkPacket(*pkt1);

    if (bigChest)
    {
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
        auto nbt2 = CompoundTag::fromSNBT(chest2);
        nbt2->putInt("x", pos.x + 1);
        nbt2->putInt("y", pos.y);
        nbt2->putInt("z", pos.z);
        nbt2->putInt("pairx", pos.x);
        nbt2->putInt("pairz", pos.z);
        nbt2->putByte("pairlead", 0);
        nbt2->putString("CustomName", title);

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
}

// 打开容器包
void chestUI::open(Player *pl, const BlockPos &pos)
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

// 被动关闭容器包
void chestUI::close(Player *pl, const BlockPos &pos,bool isSwitch)
{
    RecordedInfo[pl].forceClose = true;
    RecordedInfo[pl].isSwitch = isSwitch;
    BinaryStream bs;
    bs.writeByte(0);
    bs.writeBool(true);
    auto pkt = MinecraftPackets::createPacket(MinecraftPacketIds::ContainerClose);
    pkt->read(bs);
    pl->sendNetworkPacket(*pkt);
}

// 打开箱子UI
void chestUI::OpenContainerUI(
    Player *pl,
    string title,
    std::unordered_map<int, string> option,
    void (*func)(Player *, int),
    bool bigChest)
{
    // if(pl->getPlatform())
    auto pos = pl->getBlockPos();
    pos = pos.add(0, 5, 0);
    updateBlock(pl, pos, bigChest);
    updateBlockActor(pl, pos, title, option, bigChest);
    Schedule::delay([pl, pos]
                    { open(pl, pos); },
                    0.3);
    ContainerUIData data = {pl, option, pos, func, bigChest, false,false};
    RecordedInfo[pl] = data;
}

// 关闭箱子UI
void chestUI::CloseContainerUI(Player *pl)
{
    auto it = RecordedInfo.find(pl);
    if (it != RecordedInfo.end())
    {
        pl->sendUpdateBlockPacket(it->second.bp, pl->getBlockSource()->getBlock(it->second.bp).getRuntimeId());
        if(it->second.bigChest){
            pl->sendUpdateBlockPacket(it->second.bp.add(1, 0, 0), pl->getBlockSource()->getBlock(it->second.bp.add(1, 0, 0)).getRuntimeId());
        }
        
        RecordedInfo.erase(it);
    }
}

void chestUI::callback(Player *pl, int slot)
{
    auto it = RecordedInfo.find(pl);
    if (it != RecordedInfo.end())
    {
        (*it->second.func)(pl, slot);
    }
}