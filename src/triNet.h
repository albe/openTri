/*
 * triNet.h: Header for wifi
 * This file is part of the "tri Engine".
 *
 * Copyright (C) 2007 tri
 * Copyright (C) 2007 InsertWittyName <tias_dp@hotmail.com>
 *
 * $Id: $
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
 
#ifndef __TRINET_H__
#define __TRINET_H__

#include <psputility.h>
#include <netinet/in.h>
#include <sys/fd_set.h>

#include "triTypes.h"

/** @defgroup triNet Wifi
 *  @{
 */
 
#define TRI_SOCKET_TCP SOCK_STREAM /**< Stream socket */
#define TRI_SOCKET_UDP SOCK_DGRAM /**< Datagram socket */

#define TRI_LOCAL_IP 0 /**< Local IP ie. 192.168.x.x */
#define TRI_REAL_IP  1 /**< 'Real' IP */

#define TRI_MAX_CLIENTS 256 /**< Maximum clients that can connect to a server */

/**
  * Connection config
  */
typedef struct
{
	triUInt index; /**< Connection index */
	netData name;  /**< Connection name */
} triNetConfig;

/**
  * A socket
  */
typedef int triSocket;

/**
  * A socket set (group)
  */
typedef fd_set triSocketSet;

/**
 * Initialise the wifi
 *
 * @returns true on success else an error code.
 *
 * @note Requires kernel mode
 */
triBool triNetInit();

/**
 * Get the connection configs
 *
 * @param configs - An array of ::triNetConfig
 *
 * @param count - Size of the array
 *
 * @returns Number of configs found
 */
triUInt triNetGetConfigs(triNetConfig *configs, triUInt count);

/**
 * Get the status of the wlan switch
 *
 * @returns true if switch is up
 */
triBool triNetSwitchStatus();

/**
 * Connect to an access point
 *
 * @param config - The connection config to connect to
 *
 * @returns true on success else an error code
 */
triBool triNetConnect(triNetConfig *config);

/**
 * Disconnect from an access point
 */
triVoid triNetDisconnect();

/**
 * Check connected to an access point
 *
 * @returns true on valid connection
 */
triBool triNetIsConnected();

/**
 * Get local IP
 *
 * @returns true on success
 */
triBool triNetGetLocalIp(char *buffer);

/**
 * Resolve a host name to an IP
 *
 * @param hostname The host to resolve
 *
 * @returns The IP of the host
 */
triChar* triNetResolveHost(triChar *hostname);

/**
 * Create a socket
 *
 * @returns The socket file descripter
 */
triSocket triNetSocketCreate(void);

/**
 * Accept a new connection
 *
 * @param socket The socket to accept on
 *
 * @returns The socket file descripter of the new connection
 */
triSocket triNetSocketAccept(triSocket socket);

/**
 * Bind a socket to an address/port
 *
 * @param socket - A valid triSocket
 *
 * @param port - The port
 *
 * @returns true on success
 */
triBool triNetSocketBind(triSocket socket, triU16 port);

/**
 * Listen on a socket for incoming connections
 *
 * @param socket - A valid triSocket
 *
 * @param maxConnections - Number of connections allowed on the incoming queue
 *
 * @returns true on success
 */
triBool triNetSocketListen(triSocket socket, triUInt maxConnections);

/**
 * Connect using a socket
 *
 * @param socket - A valid triSocket
 *
 * @returns true on success
 */
triBool triNetSocketConnect(triSocket socket, char *ip, triU16 port);

/**
 * Send data using a socket
 *
 * @param socket - A valid triSocket
 *
 * @param data - The data to send
 *
 * @returns The number of bytes sent or < 0 on error
 */
triSInt triNetSocketSend(triSocket socket, const char *data, triSInt length);

/**
 * Receive data using a socket
 *
 * @param socket - A valid triSocket
 *
 * @param data - The buffer for the received data to go
 *
 * @returns The number of bytes received, 0 if remote closed connection or < 0 on error
 */
triSInt triNetSocketReceive(triSocket socket, char *data);

/**
 * Close a socket
 *
 * @param socket The socket to close
 */
triVoid triNetSocketClose(triSocket socket);

/**
 * Clear a socket set
 *
 * @param set The socket set to clear
 */
triVoid triNetSocketSetClear(triSocketSet *set);

/**
 * Add a socket to a socket set
 *
 * @param socket The socket to add
 *
 * @param set The set to add the socket to
 */
triVoid triNetSocketSetAdd(triSocket socket, triSocketSet *set);

/**
 * Remove a socket from a socket set
 *
 * @param socket The socket to remove
 *
 * @param set The set to remove the socket from
 */
triVoid triNetSocketSetRemove(triSocket socket, triSocketSet *set);

/**
 * Check for socket updates within a set
 *
 * @param socket The socket to check
 *
 * @param set The set to check within
 *
 * @returns true if socket requires receiving
 */
triBool triNetSocketSetIsMember(triSocket socket, triSocketSet *set);

/**
 * Remove a socket from a socket set
 *
 * @param socket The socket to remove
 *
 * @param set The set to remove the socket from
 */
triSInt triNetSocketSelect(triUInt maxSockets, triSocketSet *set);

/**
 * Save web 'item' (page/file etc)
 *
 * @param url - URL to connect to
 *
 * @param response - Buffer to save the response to
 *
 * @note Requires a full URL, ie. www.myhost.com/index.html, not just www.myhost.com/
 */
triVoid triNetGetUrl(triChar *url, triChar *response);

/** @} */

#endif // __TRINET_H__
