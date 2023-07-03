#pragma once
#include <llapi/Global.h>
#include <llapi/mc/Player.hpp>
#include <llapi/mc/Level.hpp>
#include <llapi/mc/BlockSource.hpp>
#include <llapi/mc/Block.hpp>
#include <llapi/mc/VanillaBlocks.hpp>
#include <llapi/mc/BinaryStream.hpp>
#include <llapi/mc/MinecraftPackets.hpp>
#include <llapi/mc/Dimension.hpp>
#include <llapi/mc/Packet.hpp>
#include <llapi/mc/ExtendedStreamReadResult.hpp>

using namespace std;

namespace update {
	inline void UpdateBlockPacket(Dimension* dim, BlockPos bp, const unsigned int runtimeId, unsigned int layer = 1) {
		if (!dim)
			return;
		BinaryStream wp;
		wp.writeVarInt(bp.x);
		wp.writeUnsignedVarInt(bp.y);
		wp.writeVarInt(bp.z);
		wp.writeUnsignedVarInt(runtimeId);
		wp.writeUnsignedVarInt(3);  // flag
		wp.writeUnsignedVarInt(layer);  // layer
		shared_ptr<Packet> pkt = MinecraftPackets::createPacket(MinecraftPacketIds::UpdateBlock);
		pkt->read(wp);
		dim->sendPacketForPosition({ bp.x, bp.y, bp.z }, *pkt, nullptr);
	};

	inline void UpdateBlockPacket(int dimId, BlockPos bp, const unsigned int runtimeId, unsigned int layer = 1) {
		auto dim = (Dimension*)Global<Level>->getDimension(dimId).mHandle.lock().get();
		UpdateBlockPacket(dim, bp, runtimeId);
	};
}