/*
* Copyright (c) 2013-2024, The PurpleI2P Project
*
* This file is part of Purple i2pd project and licensed under BSD3
*
* See full license text in LICENSE file at top of project tree
*/

#ifndef NETDB_REQUESTS_H__
#define NETDB_REQUESTS_H__

#include <inttypes.h>
#include <memory>
#include <set>
#include <unordered_map>
#include "Identity.h"
#include "RouterInfo.h"
#include "util.h"

namespace i2p
{
namespace data
{
	const size_t MAX_NUM_REQUEST_ATTEMPTS = 5;
	const uint64_t MANAGE_REQUESTS_INTERVAL = 1; // in seconds
	const uint64_t MIN_REQUEST_TIME = 5; // in seconds
	const uint64_t MAX_REQUEST_TIME = MAX_NUM_REQUEST_ATTEMPTS * (MIN_REQUEST_TIME + MANAGE_REQUESTS_INTERVAL);
	const uint64_t MAX_EXPLORATORY_REQUEST_TIME = 30; // in seconds
	const uint64_t REQUEST_CACHE_TIME = MAX_REQUEST_TIME + 40; // in seconds
	const uint64_t REQUESTED_DESTINATIONS_POOL_CLEANUP_INTERVAL = 191; // in seconds
	
	class RequestedDestination
	{
		public:

			typedef std::function<void (std::shared_ptr<RouterInfo>)> RequestComplete;

			RequestedDestination (const IdentHash& destination, bool isExploratory = false, bool direct = true);
			~RequestedDestination ();

			const IdentHash& GetDestination () const { return m_Destination; };
			size_t GetNumExcludedPeers () const;
			std::set<IdentHash> GetExcludedPeers () const;
			void ClearExcludedPeers ();
			bool IsExploratory () const { return m_IsExploratory; };
			bool IsDirect () const { return m_IsDirect; };
			bool IsActive () const { return m_IsActive; };
			bool IsExcluded (const IdentHash& ident) const;
			uint64_t GetCreationTime () const { return m_CreationTime; };
			uint64_t GetLastRequestTime () const { return m_LastRequestTime; };
			std::shared_ptr<I2NPMessage> CreateRequestMessage (std::shared_ptr<const RouterInfo>, std::shared_ptr<const i2p::tunnel::InboundTunnel> replyTunnel);
			std::shared_ptr<I2NPMessage> CreateRequestMessage (const IdentHash& floodfill);

			void SetRequestComplete (const RequestComplete& requestComplete) { m_RequestComplete = requestComplete; };
			RequestComplete GetRequestComplete () const { return m_RequestComplete; };
			bool IsRequestComplete () const { return m_RequestComplete != nullptr; };
			void Success (std::shared_ptr<RouterInfo> r);
			void Fail ();

		private:

			IdentHash m_Destination;
			bool m_IsExploratory, m_IsDirect, m_IsActive;
			mutable std::mutex m_ExcludedPeersMutex;
			std::set<IdentHash> m_ExcludedPeers;
			uint64_t m_CreationTime, m_LastRequestTime; // in seconds
			RequestComplete m_RequestComplete;
	};

	class NetDbRequests
	{
		public:

			void Start ();
			void Stop ();

			std::shared_ptr<RequestedDestination> CreateRequest (const IdentHash& destination, bool isExploratory, 
				bool direct = false, RequestedDestination::RequestComplete requestComplete = nullptr);
			void RequestComplete (const IdentHash& ident, std::shared_ptr<RouterInfo> r);
			std::shared_ptr<RequestedDestination> FindRequest (const IdentHash& ident) const;
			void ManageRequests ();
			bool SendNextRequest (std::shared_ptr<RequestedDestination> dest);
			
		private:

			mutable std::mutex m_RequestedDestinationsMutex;
			std::unordered_map<IdentHash, std::shared_ptr<RequestedDestination> > m_RequestedDestinations;
			i2p::util::MemoryPoolMt<RequestedDestination> m_RequestedDestinationsPool;
			uint64_t m_LastPoolCleanUpTime = 0; // in seconds
	};
}
}

#endif
