/*
 * triNet.c: Code for wifi
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
 
#include <pspkernel.h>
#include <pspsdk.h>
#include <pspwlan.h>
#include <string.h>
#include <pspnet.h>
#include <pspnet_apctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <errno.h>
#include <stdio.h>

#include "triNet.h"
#include "triLog.h"
#include "triTypes.h"
#include "triMemory.h"

typedef struct _url
{
	triChar host[256];
	triChar page[256];
	triChar getString[256];
	triChar hostString[256];
} _url;

triBool triNetInit()
{
	triLogPrint("triNet: Loading needed modules\r\n");

	int result = sceUtilityLoadNetModule(PSP_NET_MODULE_COMMON);
	
	if( result != 0 )
	{
		triLogError("triNet: Error loading module\r\n");
		return(0); // Error
	}
		
	result = sceUtilityLoadNetModule(PSP_NET_MODULE_INET);
	
	if( result != 0 )
	{
		triLogError("triNet: Error loading module\r\n");
		return(0); // Error
	}

	triLogPrint("triNet: Modules loaded successfully\r\n");

	return(1);
}

triUInt triNetGetConfigs( triNetConfig *configs, triUInt count )
{
	triLogPrint("triNet: Getting connection configs\r\n");

	triUInt loop;
	triUInt configCount = 0;

	for ( loop = 1; loop <= count; loop++ ) // skip the 0th connection
	{
		if ( sceUtilityCheckNetParam( loop ) != 0 )
			break;  // no more
		configs[configCount].name.asUint = 0xBADF00D;
		memset( &configs[configCount].name.asString[4], 0, 124 );
		sceUtilityGetNetParam( loop, 0, &configs[configCount].name );
		configs[configCount].index = loop;
		configCount++;
	}

        triLogPrint("triNet: %i connection configs found\r\n", configCount);

	return configCount;
}

triBool triNetSwitchStatus()
{
	return sceWlanGetSwitchState();
}


triBool triNetConnect( triNetConfig *config )
{
	triSInt err;
	triSInt stateLast = -1;

	err = pspSdkInetInit();
	
        if ( err != 0 )
	{
		triLogError("triNet: pspSdkInetInit returns %08X\r\n", err);
		return(0);
	}

	err = sceNetApctlConnect( config->index );

	if ( err != 0 )
	{
		triLogError("triNet: sceNetApctlConnect returns %08X\r\n", err);
		return(0);
	}

	triLogPrint("triNet: Connecting...\n");

	while ( 1 )
	{
		triSInt state;
		err = sceNetApctlGetState(&state);
		if ( err != 0 )
		{
			triLogError("triNet: sceNetApctlGetState returns %08X\r\n", err);
			break;
		}
		if ( state > stateLast )
		{
			triLogPrint("triNet: Connection state %d of 4\r\n", state);
			stateLast = state;
		}
		if ( state == 4 )
			break;  // connected with static IP

		// wait a little before polling again
		sceKernelDelayThread(50*1000); // 50ms
	}

	triLogPrint("triNet: Connected\r\n");

	if( err != 0 )
	{
		triLogError("triNet: Connection error: %08X\r\n", err);
		return(0);
	}
	
	triNetIsConnected();

	return(1);
}

triVoid triNetDisconnect()
{
	
	triLogPrint("triNet: Disconnecting\r\n");

	sceNetApctlDisconnect();
	pspSdkInetTerm();
}

triBool triNetIsConnected()
{
        triChar szMyIPAddr[32];

        if (sceNetApctlGetInfo(8, szMyIPAddr) != 0)
		return(0);
		
	triLogPrint("triNet: Local IP is %s\r\n", szMyIPAddr);

        return(1);
}

triBool triNetGetLocalIp(char *buffer)
{
        if (sceNetApctlGetInfo(8, buffer) != 0)
		return(0);
		
	triLogPrint("triNet: Local IP is %s\r\n", buffer);

        return(1);
}

triChar* triNetResolveHost(triChar *hostname)
{
	triLogPrint("triNet: Resolving address: %s\r\n", hostname);

	struct hostent *h;

	if(!(h = gethostbyname(hostname)))
	{
		triLogError("triNet: Error resolving %s\r\n", hostname);
		return NULL;
	}

	triLogPrint("triNet: Resolved %s to %s\r\n", hostname, inet_ntoa(*((struct in_addr *)h->h_addr)));

	return inet_ntoa(*((struct in_addr *)h->h_addr));
}

triSocket triNetSocketCreate(void)
{
	triSocket sock;

	sock = socket(PF_INET, SOCK_STREAM, 0);
	
	if(sock < 0)
	{
		triLogError("triNet: Socket create failed\r\n");
		return -1;
	}
	
	return sock;
}

triSocket triNetSocketAccept(triSocket socket)
{
	triSocket sock;
	
	struct sockaddr_in client;

	size_t size;

	sock = accept(socket, (struct sockaddr *) &client, &size);
	
	if(sock < 0)
	{
		triLogError("triNet: Error in socket accept: %s\r\n", strerror(errno));
		return -1;
	}
	
	return sock;
}

triBool triNetSocketBind(triSocket socket, triU16 port)
{
	struct sockaddr_in theAddr;

        theAddr.sin_family = AF_INET;    // host byte order
	theAddr.sin_port = htons(port);  // short, network byte order
	theAddr.sin_addr.s_addr = 0; // Local IP

	int error;

	error = bind(socket, (struct sockaddr *)&theAddr, sizeof(struct sockaddr));

	if(error < 0)
	{
		triLogError("triNet: Bind failed\r\n");
		return 0;
	}
	
	return 1;
}

triBool triNetSocketListen(triSocket socket, triUInt maxConnections)
{
	int error;
	
	error = listen(socket, maxConnections);
	
	if(error < 0)
	{
		triLogError("triNet: Listen failed\r\n");
		return 0;
	}
	
	return 1;
}

triBool triNetSocketConnect(triSocket socket, char *ip, triU16 port)
{
	triLogPrint("triNet: Connecting\r\n");
	
	struct sockaddr_in theAddr;

        theAddr.sin_family = AF_INET;    // host byte order
	theAddr.sin_port = htons(port);  // short, network byte order
	inet_aton(ip, &(theAddr.sin_addr));
	memset(&(theAddr.sin_zero), '\0', 8);  // zero the rest of the struct

	if(connect(socket, (struct sockaddr *)&theAddr, sizeof(theAddr)) < 0)
	{
		triLogError("triNet: Connect failed\r\n");
		return 0;
	}
	
	return 1;
}

triSInt triNetSocketSend(triSocket socket, const char *data, triSInt length)
{
	triLogPrint("triNet: Sending data\r\n");

	triUInt bytesSent = send(socket, data, length, 0);
	
	if(bytesSent < 0)
	{
		triLogError("triNet: Error on send\r\n");
	}
	
	if(bytesSent == 0)
	{
		triLogError("triNet: Socket %d closed connection\r\n", socket);
	}

	return bytesSent;
}

triSInt triNetSocketReceive(triSocket socket, char *data)
{
	triSInt bytesRecv;

	bytesRecv = recv(socket, data, 1023, 0);
	
	if(bytesRecv == 0)
	{
		triLogError("triNet: Remote closed the connection on receive\r\n");
	}
	
	if(bytesRecv < 0)
	{
		triLogError("triNet: Error on receive\r\n");
	}

	return bytesRecv;
}

triVoid triNetSocketClose(triSocket socket)
{
	close(socket);
}

triVoid triNetSocketSetClear(triSocketSet *set)
{
	FD_ZERO(set);
}

triVoid triNetSocketSetAdd(triSocket socket, triSocketSet *set)
{
	FD_SET(socket, set);
}

triVoid triNetSocketSetRemove(triSocket socket, triSocketSet *set)
{
	FD_CLR(socket, set);
}

triBool triNetSocketSetIsMember(triSocket socket, triSocketSet *set)
{
	if(FD_ISSET(socket, set))
		return 1;
	
	return 0;
}

triSInt triNetSocketSelect(triUInt maxSockets, triSocketSet *set)
{
	// TODO: Add argument manipulation of timeout etc.
	
	triSInt numSockets;

	struct timeval tv;
	
	tv.tv_sec = 0;
	tv.tv_usec = 1000;

	numSockets = select(maxSockets, set, NULL, NULL, &tv);

	if(numSockets < 0)
	{
		triLogError("triNet: Error on select\r\n");
		return -1;
	}
	
	return numSockets;
}

_url _parseUrl(char *theUrl)
{
	// Bit messy....
	_url parsedUrl;
	triChar *host;
	triChar *page;
	triChar *check;
	triChar urlBuffer[256];

	memset(&parsedUrl, 0, sizeof(_url));
	
	// Check for http://
	check = strstr(theUrl, "http://");

	if(check)
	{
		check += 7;
		strcpy(theUrl, check);
	}
	
	strcpy(urlBuffer, theUrl);

	// Get first '/'
	host = strtok(urlBuffer, "/");
	strcpy(parsedUrl.host, host);

        page = strstr(theUrl, "/");
        strcpy(parsedUrl.page, page);

	// For debugging
	//triLogPrint("theUrl: %s\r\n", theUrl);
	//triLogPrint("Host: %s\r\n", parsedUrl.host);
	//triLogPrint("Page: %s\r\n", parsedUrl.page);

	return parsedUrl;
}

triVoid triNetGetUrl(triChar *url, triChar *response)
{
	triLogPrint("triNet: Getting URL %s\r\n", url);

	triChar buffer[1024];
	
	_url parsedUrl;
	
	parsedUrl = _parseUrl(url);
	
	sprintf(parsedUrl.getString, "GET %s HTTP/1.0\r\n", parsedUrl.page);
	sprintf(parsedUrl.hostString, "host: %s\r\n\r\n", parsedUrl.host);
        
        triSocket mySocket = triNetSocketCreate();

        triChar *theHost = triNetResolveHost(parsedUrl.host);

        triNetSocketConnect(mySocket, theHost, 80);

        triNetSocketSend(mySocket, parsedUrl.getString, strlen(parsedUrl.getString));
        triNetSocketSend(mySocket, parsedUrl.hostString, strlen(parsedUrl.hostString));

        triSInt bytesReceived;
        
        triChar header[1024];

        memset(header, 0, sizeof(header));
        memset(buffer, 0, sizeof(buffer));

        triSInt headerFinished = 0;

        triChar *headerEnd;

	while(1)
	{
		bytesReceived = triNetSocketReceive(mySocket, buffer);

		if(strlen(buffer) > 0)
		{
			if(headerFinished)
			{
				strcpy(response, headerEnd);
				break;
			}
			else
			{
				strcat(header, buffer);
				headerEnd = strstr(header, "\r\n\r\n");
				
				if(headerEnd)
				{
					headerEnd += 4;
					headerFinished = 1;
					strcpy(response, headerEnd);
					break;
				}
			}
		}
	}
}
