// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef MULTIVERSE_DBP_UTILS_H
#define MULTIVERSE_DBP_UTILS_H

#include <arpa/inet.h>
#include <cstring>
#include <random>
#include <vector>

#include "dbp.pb.h"
#include "lws.pb.h"
#include "sn.pb.h"

namespace multiverse
{
class CDbpUtils
{
public:
    static uint32_t ParseLenFromMsgHeader(const char *header, int size)
    {
        uint32_t lenNetWorkOrder = 0;
        std::memcpy(&lenNetWorkOrder, header, 4);
        return ntohl(lenNetWorkOrder);
    }

    static void WriteLenToMsgHeader(uint32_t len, char *header, int size)
    {
        uint32_t lenNetWorkOrder = htonl(len);
        std::memcpy(header, &lenNetWorkOrder, 4);
    }

    static uint64 CurrentUTC()
    {
        boost::posix_time::ptime epoch(boost::gregorian::date(1970, boost::gregorian::Jan, 1));
        boost::posix_time::time_duration time_from_epoch =
            boost::posix_time::second_clock::universal_time() - epoch;
        return time_from_epoch.total_seconds();
    }

    static std::string RandomString()
    {
        static auto &chrs = "0123456789"
                            "abcdefghijklmnopqrstuvwxyz"
                            "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

        thread_local static std::mt19937 rg{std::random_device{}()};
        thread_local static std::uniform_int_distribution<std::string::size_type> pick(0, sizeof(chrs) - 2);

        std::string s;

        int length = 20;

        s.reserve(length);

        while (length--)
            s += chrs[pick(rg)];

        return s;
    }

    static std::vector<std::string> Split(const std::string& str, char delim)
    {
        std::vector<std::string> ret;
        std::istringstream ss(str);
        std::string s;    
        while (std::getline(ss, s, ';')) 
        {
            ret.push_back(s);
        }
        return ret;
    }

    static void DbpToLwsTransaction(const CMvDbpTransaction* dbptx, lws::Transaction* tx)
    {
        tx->set_nversion(dbptx->nVersion);
        tx->set_ntype(dbptx->nType);
        tx->set_nlockuntil(dbptx->nLockUntil);

        std::string hashAnchor(dbptx->hashAnchor.begin(), dbptx->hashAnchor.end());
        tx->set_hashanchor(hashAnchor);

        for (const auto& input : dbptx->vInput)
        {
            std::string inputHash(input.hash.begin(), input.hash.end());
            auto add = tx->add_vinput();
            add->set_hash(inputHash);
            add->set_n(input.n);
        }

        lws::Transaction::CDestination* dest = new lws::Transaction::CDestination();
        dest->set_prefix(dbptx->cDestination.prefix);
        dest->set_size(dbptx->cDestination.size);

        std::string destData(dbptx->cDestination.data.begin(), dbptx->cDestination.data.end());
        dest->set_data(destData);
        tx->set_allocated_cdestination(dest);

        tx->set_namount(dbptx->nAmount);
        tx->set_ntxfee(dbptx->nTxFee);

        std::string mintVchData(dbptx->vchData.begin(), dbptx->vchData.end());
        tx->set_vchdata(mintVchData);

        std::string mintVchSig(dbptx->vchSig.begin(), dbptx->vchSig.end());
        tx->set_vchsig(mintVchSig);

        std::string hash(dbptx->hash.begin(), dbptx->hash.end());
        tx->set_hash(hash);
        tx->set_nchange(dbptx->nChange);
    }

    static void LwsToDbpTransaction(const lws::Transaction* tx, CMvDbpTransaction* dbptx)
    {
        dbptx->nVersion = tx->nversion();
        dbptx->nType = tx->ntype();
        dbptx->nLockUntil = tx->nlockuntil();

        std::vector<unsigned char> hashAnchor(tx->hashanchor().begin(), tx->hashanchor().end());
        dbptx->hashAnchor = hashAnchor;

        for(int i = 0; i < tx->vinput_size(); ++i)
        {
            auto input = tx->vinput(i);
            CMvDbpTxIn txin;
            txin.hash = std::vector<unsigned char>(input.hash().begin(), input.hash().end());
            txin.n = input.n();

            dbptx->vInput.push_back(txin);
        }

        dbptx->cDestination.prefix = tx->cdestination().prefix();
        dbptx->cDestination.size = tx->cdestination().size();
        auto& data = tx->cdestination().data();
        dbptx->cDestination.data = std::vector<unsigned char>(data.begin(),data.end());

        dbptx->nAmount = tx->namount();
        dbptx->nTxFee = tx->ntxfee();

        dbptx->vchData = std::vector<unsigned char>(tx->vchdata().begin(), tx->vchdata().end());
        dbptx->vchSig = std::vector<unsigned char>(tx->vchsig().begin(), tx->vchsig().end());

        dbptx->hash = std::vector<unsigned char>(tx->hash().begin(), tx->hash().end());

        dbptx->nChange = tx->nchange();

    }

    static void DbpToLwsBlock(const CMvDbpBlock* pBlock, lws::Block& block)
    {
        block.set_nversion(pBlock->nVersion);
        block.set_ntype(pBlock->nType);
        block.set_ntimestamp(pBlock->nTimeStamp);

        std::string hashPrev(pBlock->hashPrev.begin(), pBlock->hashPrev.end());
        block.set_hashprev(hashPrev);

        std::string hashMerkle(pBlock->hashMerkle.begin(), pBlock->hashMerkle.end());
        block.set_hashmerkle(hashMerkle);

        std::string vchproof(pBlock->vchProof.begin(), pBlock->vchProof.end());
        block.set_vchproof(vchproof);

        std::string vchSig(pBlock->vchSig.begin(), pBlock->vchSig.end());
        block.set_vchsig(vchSig);

        //txMint
        lws::Transaction* txMint = new lws::Transaction();
        DbpToLwsTransaction(&(pBlock->txMint), txMint);
        block.set_allocated_txmint(txMint);

        //repeated vtx
        for (const auto& tx : pBlock->vtx)
        {
            DbpToLwsTransaction(&tx, block.add_vtx());
        }

        block.set_nheight(pBlock->nHeight);
        std::string hash(pBlock->hash.begin(), pBlock->hash.end());
        block.set_hash(hash);
    }



    static void LwsToDbpBlock(const lws::Block *pBlock, CMvDbpBlock& block)
    {
        block.nVersion = pBlock->nversion();
        block.nType = pBlock->ntype();
        block.nTimeStamp = pBlock->ntimestamp();

        block.hashPrev = std::vector<unsigned char>(pBlock->hashprev().begin(),pBlock->hashprev().end());
        block.hashMerkle = std::vector<unsigned char>(pBlock->hashmerkle().begin(),pBlock->hashmerkle().end());
        block.vchProof = std::vector<unsigned char>(pBlock->vchproof().begin(), pBlock->vchproof().end());
        block.vchSig = std::vector<unsigned char>(pBlock->vchsig().begin(), pBlock->vchsig().end());

        //txMint
        LwsToDbpTransaction(&(pBlock->txmint()), &(block.txMint));

        //repeated vtx
        for(int i = 0; i < pBlock->vtx_size(); ++i)
        {
            auto vtx = pBlock->vtx(i);
            CMvDbpTransaction tx;
            LwsToDbpTransaction(&vtx,&tx);
            block.vtx.push_back(tx);
        }

        block.nHeight = pBlock->nheight();
        block.hash = std::vector<unsigned char>(pBlock->hash().begin(), pBlock->hash().end());

    }

    static void DbpToSnTransaction(const CMvDbpTransaction* dbptx, sn::Transaction* tx)
    {
        tx->set_nversion(dbptx->nVersion);
        tx->set_ntype(dbptx->nType);
        tx->set_nlockuntil(dbptx->nLockUntil);

        std::string hashAnchor(dbptx->hashAnchor.begin(), dbptx->hashAnchor.end());
        tx->set_hashanchor(hashAnchor);

        for (const auto& input : dbptx->vInput)
        {
            std::string inputHash(input.hash.begin(), input.hash.end());
            auto add = tx->add_vinput();
            add->set_hash(inputHash);
            add->set_n(input.n);
        }

        sn::Transaction::CDestination* dest = new sn::Transaction::CDestination();
        dest->set_prefix(dbptx->cDestination.prefix);
        dest->set_size(dbptx->cDestination.size);

        std::string destData(dbptx->cDestination.data.begin(), dbptx->cDestination.data.end());
        dest->set_data(destData);
        tx->set_allocated_cdestination(dest);

        tx->set_namount(dbptx->nAmount);
        tx->set_ntxfee(dbptx->nTxFee);

        std::string mintVchData(dbptx->vchData.begin(), dbptx->vchData.end());
        tx->set_vchdata(mintVchData);

        std::string mintVchSig(dbptx->vchSig.begin(), dbptx->vchSig.end());
        tx->set_vchsig(mintVchSig);

        std::string hash(dbptx->hash.begin(), dbptx->hash.end());
        tx->set_hash(hash);
        std::string fork(dbptx->fork.begin(), dbptx->fork.end());
        tx->set_fork(fork);
    }

    static void DbpToSnBlock(const CMvDbpBlock* pBlock, sn::Block& block)
    {
        block.set_nversion(pBlock->nVersion);
        block.set_ntype(pBlock->nType);
        block.set_ntimestamp(pBlock->nTimeStamp);

        std::string hashPrev(pBlock->hashPrev.begin(), pBlock->hashPrev.end());
        block.set_hashprev(hashPrev);

        std::string hashMerkle(pBlock->hashMerkle.begin(), pBlock->hashMerkle.end());
        block.set_hashmerkle(hashMerkle);

        std::string vchproof(pBlock->vchProof.begin(), pBlock->vchProof.end());
        block.set_vchproof(vchproof);

        std::string vchSig(pBlock->vchSig.begin(), pBlock->vchSig.end());
        block.set_vchsig(vchSig);

        //txMint
        sn::Transaction* txMint = new sn::Transaction();
        DbpToSnTransaction(&(pBlock->txMint), txMint);
        block.set_allocated_txmint(txMint);

        //repeated vtx
        for (const auto& tx : pBlock->vtx)
        {
            DbpToSnTransaction(&tx, block.add_vtx());
        }

        block.set_nheight(pBlock->nHeight);
        std::string hash(pBlock->hash.begin(), pBlock->hash.end());
        block.set_hash(hash);
        std::string fork(pBlock->fork.begin(), pBlock->fork.end());
        block.set_fork(fork);
    }

    static void SnToDbpTransaction(const sn::Transaction* tx, CMvDbpTransaction* dbptx)
    {
        dbptx->nVersion = tx->nversion();
        dbptx->nType = tx->ntype();
        dbptx->nLockUntil = tx->nlockuntil();

        std::vector<unsigned char> hashAnchor(tx->hashanchor().begin(), tx->hashanchor().end());
        dbptx->hashAnchor = hashAnchor;

        for(int i = 0; i < tx->vinput_size(); ++i)
        {
            auto input = tx->vinput(i);
            CMvDbpTxIn txin;
            txin.hash = std::vector<unsigned char>(input.hash().begin(), input.hash().end());
            txin.n = input.n();

            dbptx->vInput.push_back(txin);
        }

        dbptx->cDestination.prefix = tx->cdestination().prefix();
        dbptx->cDestination.size = tx->cdestination().size();
        auto& data = tx->cdestination().data();
        dbptx->cDestination.data = std::vector<unsigned char>(data.begin(),data.end());

        dbptx->nAmount = tx->namount();
        dbptx->nTxFee = tx->ntxfee();

        dbptx->vchData = std::vector<unsigned char>(tx->vchdata().begin(), tx->vchdata().end());
        dbptx->vchSig = std::vector<unsigned char>(tx->vchsig().begin(), tx->vchsig().end());

        dbptx->hash = std::vector<unsigned char>(tx->hash().begin(), tx->hash().end());
        dbptx->fork = std::vector<unsigned char>(tx->fork().begin(), tx->fork().end());
    }

    static void SnToDbpBlock(const sn::Block *pBlock, CMvDbpBlock& block)
    {
        block.nVersion = pBlock->nversion();
        block.nType = pBlock->ntype();
        block.nTimeStamp = pBlock->ntimestamp();

        block.hashPrev = std::vector<unsigned char>(pBlock->hashprev().begin(),pBlock->hashprev().end());
        block.hashMerkle = std::vector<unsigned char>(pBlock->hashmerkle().begin(),pBlock->hashmerkle().end());
        block.vchProof = std::vector<unsigned char>(pBlock->vchproof().begin(), pBlock->vchproof().end());
        block.vchSig = std::vector<unsigned char>(pBlock->vchsig().begin(), pBlock->vchsig().end());

        //txMint
        SnToDbpTransaction(&(pBlock->txmint()), &(block.txMint));

        //repeated vtx
        for(int i = 0; i < pBlock->vtx_size(); ++i)
        {
            auto vtx = pBlock->vtx(i);
            CMvDbpTransaction tx;
            SnToDbpTransaction(&vtx,&tx);
            block.vtx.push_back(tx);
        }

        block.nHeight = pBlock->nheight();
        block.hash = std::vector<unsigned char>(pBlock->hash().begin(), pBlock->hash().end());
        block.fork = std::vector<unsigned char>(pBlock->fork().begin(), pBlock->fork().end());

    }

    static void DbpToSnSysCmd(const CMvDbpSysCmd* pCmd, sn::SysCmd& cmd)
    {
        cmd.set_id(pCmd->id);
        std::string fork(pCmd->fork.begin(), pCmd->fork.end());
        cmd.set_forkid(fork);
        cmd.set_cmd(pCmd->nCmd);

        for(const auto& arg : pCmd->args)
        {
            cmd.add_arg(arg);
        }
    }
    
    static void SnToDbpSysCmd(const sn::SysCmd* pCmd, CMvDbpSysCmd& cmd)
    {
        cmd.id = pCmd->id();
        cmd.fork = std::vector<uint8>(pCmd->forkid().begin(), pCmd->forkid().end());
        cmd.nCmd = pCmd->cmd();

        for(int i = 0; i < pCmd->arg_size(); ++i)
        {
            cmd.args.push_back(pCmd->arg(i));
        }
    }

    static void DbpToSnBlockCmd(const CMvDbpBlockCmd* pCmd, sn::BlockCmd& cmd)
    {
        cmd.set_id(pCmd->id);
        std::string fork(pCmd->fork.begin(), pCmd->fork.end());
        cmd.set_forkid(fork);
        std::string hash(pCmd->hash.begin(), pCmd->hash.end());
        cmd.set_hash(hash);
    }

    static void SnToDbpBlockCmd(const sn::BlockCmd* pCmd, CMvDbpBlockCmd& cmd)
    {
        cmd.id = pCmd->id();
        cmd.fork = std::vector<uint8>(pCmd->forkid().begin(), pCmd->forkid().end());
        cmd.hash = std::vector<uint8>(pCmd->hash().begin(), pCmd->hash().end());
    }
    
    static void DbpToSnTxCmd(const CMvDbpTxCmd* pCmd, sn::TxCmd& cmd)
    {
        cmd.set_id(pCmd->id);
        std::string fork(pCmd->fork.begin(), pCmd->fork.end());
        cmd.set_forkid(fork);
        std::string hash(pCmd->hash.begin(), pCmd->hash.end());
        cmd.set_hash(hash);
    }

    static void SnToDbpTxCmd(const sn::TxCmd* pCmd, CMvDbpTxCmd& cmd)
    {
        cmd.id = pCmd->id();
        cmd.fork = std::vector<uint8>(pCmd->forkid().begin(), pCmd->forkid().end());
        cmd.hash = std::vector<uint8>(pCmd->hash().begin(), pCmd->hash().end());
    }

    static void RawToDbpTransaction(const CTransaction& tx, const uint256& forkHash, int64 nChange, CMvDbpTransaction& dbptx)
    {
        dbptx.nVersion = tx.nVersion;
        dbptx.nType = tx.nType;
        dbptx.nLockUntil = tx.nLockUntil;

        walleve::CWalleveODataStream hashAnchorStream(dbptx.hashAnchor);
        tx.hashAnchor.ToDataStream(hashAnchorStream);

        for (const auto& input : tx.vInput)
        {
            CMvDbpTxIn txin;
            txin.n = input.prevout.n;

            walleve::CWalleveODataStream txInHashStream(txin.hash);
            input.prevout.hash.ToDataStream(txInHashStream);

            dbptx.vInput.push_back(txin);
        }

        dbptx.cDestination.prefix = tx.sendTo.prefix;
        dbptx.cDestination.size = tx.sendTo.DESTINATION_SIZE;

        walleve::CWalleveODataStream sendtoStream(dbptx.cDestination.data);
        tx.sendTo.data.ToDataStream(sendtoStream);

        dbptx.nAmount = tx.nAmount;
        dbptx.nTxFee = tx.nTxFee;
        dbptx.nChange = nChange;

        dbptx.vchData = tx.vchData;
        dbptx.vchSig = tx.vchSig;

        walleve::CWalleveODataStream hashStream(dbptx.hash);
        tx.GetHash().ToDataStream(hashStream);

        walleve::CWalleveODataStream forkStream(dbptx.fork);
        forkHash.ToDataStream(forkStream);
    }

    static void RawToDbpBlock(const CBlockEx& blockDetail, const uint256& forkHash,
                                 int blockHeight, CMvDbpBlock& block)
    {
        block.nVersion = blockDetail.nVersion;
        block.nType = blockDetail.nType;
        block.nTimeStamp = blockDetail.nTimeStamp;

        walleve::CWalleveODataStream hashPrevStream(block.hashPrev);
        blockDetail.hashPrev.ToDataStream(hashPrevStream);

        walleve::CWalleveODataStream hashMerkleStream(block.hashMerkle);
        blockDetail.hashMerkle.ToDataStream(hashMerkleStream);

        block.vchProof = blockDetail.vchProof;
        block.vchSig = blockDetail.vchSig;

        // txMint
        RawToDbpTransaction(blockDetail.txMint, forkHash, blockDetail.txMint.GetChange(0), block.txMint);

        // vtx
        int k = 0;
        for (const auto& tx : blockDetail.vtx)
        {
            CMvDbpTransaction dbpTx;
            int64 nValueIn = blockDetail.vTxContxt[k++].GetValueIn();
            RawToDbpTransaction(tx, forkHash, tx.GetChange(nValueIn), dbpTx);
            block.vtx.push_back(dbpTx);
        }

        block.nHeight = blockHeight;
        walleve::CWalleveODataStream hashStream(block.hash);
        blockDetail.GetHash().ToDataStream(hashStream);

        walleve::CWalleveODataStream forkStream(block.fork);
        forkHash.ToDataStream(forkStream);
    }

    static void DbpToRawTransaction(const CMvDbpTransaction& dbpTx, CTransaction& tx)
    {
        tx.nVersion = dbpTx.nVersion;
        tx.nType = dbpTx.nType;
        tx.nLockUntil = dbpTx.nLockUntil;

        tx.hashAnchor = uint256(dbpTx.hashAnchor);

        for (const auto& input : dbpTx.vInput)
        {
            CTxIn txin;
            txin.prevout.n = input.n;
            txin.prevout.hash = uint256(input.hash);
        
            tx.vInput.push_back(txin);
        }

        tx.sendTo.prefix = dbpTx.cDestination.prefix;
        tx.sendTo.data = uint256(dbpTx.cDestination.data);

        tx.nAmount = dbpTx.nAmount;
        tx.nTxFee = dbpTx.nTxFee;

        tx.vchData  = dbpTx.vchData;
        tx.vchSig = dbpTx.vchSig;

    }
    
    static void DbpToRawBlock(const CMvDbpBlock& dbpBlock, CBlockEx& block)
    {
        block.nVersion = dbpBlock.nVersion;
        block.nType = dbpBlock.nType;
        block.nTimeStamp = dbpBlock.nTimeStamp;

        block.hashPrev = uint256(dbpBlock.hashPrev);
        block.hashMerkle = uint256(dbpBlock.hashMerkle);

        block.vchProof = dbpBlock.vchProof;
        block.vchSig = dbpBlock.vchSig;

        // txMint
        DbpToRawTransaction(dbpBlock.txMint, block.txMint);

        // vtx
        for(const auto& dbptx : dbpBlock.vtx)
        {
            CTransaction tx;
            DbpToRawTransaction(dbptx,tx);
            block.vtx.push_back(tx);
        }

    }

};
} // namespace multiverse
#endif // MULTIVERSE_DBP_UTILS_H
