////////////////////////////////////////////////////////////
// NOTE: To operate as a server, you must
// have port 55555 open!
////////////////////////////////////////////////////////////

#include <pspkernel.h>
#include <arpa/inet.h>
#include <string.h>
#include <pspctrl.h>

#include "../../triTypes.h"
#include "../../triLog.h"
#include "../../triMemory.h"
#include "../../triInput.h"
#include "../../triNet.h"

PSP_MODULE_INFO("triNetTest", PSP_MODULE_USER, 1, 1);
PSP_HEAP_SIZE_KB(20480);

/* Exit callback */
int exit_callback(int arg1, int arg2, void *common)
{
	sceKernelExitGame();
	return 0;
}

/* Callback thread */
int CallbackThread(SceSize args, void *argp)
{
	int cbid;

	cbid = sceKernelCreateCallback("Exit Callback", exit_callback, NULL);
	sceKernelRegisterExitCallback(cbid);
	sceKernelSleepThreadCB();

	return 0;
}

/* Sets up the callback thread and returns its thread id */
int SetupCallbacks(void)
{
	int thid = 0;

	thid = sceKernelCreateThread("update_thread", CallbackThread,
				     0x11, 0xFA0, PSP_THREAD_ATTR_USER, 0);
	if(thid >= 0)
	{
		sceKernelStartThread(thid, 0, 0);
	}

	return thid;
}

int main(int argc, char **argv)
{
		SetupCallbacks();

		pspDebugScreenInit();

		triLogInit();
	
		triNetInit();

		triMemoryInit();

       while(!triNetSwitchStatus())
       {
         pspDebugScreenSetXY(0, 0);
         pspDebugScreenPrintf("Turn the wifi switch on!\n");
       }

       triNetConfig netConfigs[6];

       triSInt numConfigs = triNetGetConfigs(netConfigs, 6);
       
       if( numConfigs == 0 )
       {
	      pspDebugScreenPrintf("No connections found.\n");
	      return 0;
	}

       triUInt i;
       
       for ( i = 0; i < numConfigs; i++ )
       {
       	     pspDebugScreenPrintf("Index: %i, Name: %s\n", netConfigs[i].index, netConfigs[i].name.asString);
       }
       
       pspDebugScreenPrintf("Connecting to first config\n");
       triNetConnect(&netConfigs[0]);

       pspDebugScreenPrintf("Connected!\n");

       triChar response[1024];

       triNetGetUrl("http://insomniac.0x89.org/test/join.php", response);

       int isServer;
       
       char *server = strstr(response, "SERVER");
       
       if(server)
       		isServer = 1;
       else
       		isServer = 0;

       // Get Ip
       char *ip = strtok(response, " ");

       pspDebugScreenPrintf("My IP: %s Server?: %d\n", ip, isServer);

       if(isServer)
       {
       triSocket listener = triNetSocketCreate();

       triSocket newSocket;

       triNetSocketBind(listener, 55555);
       
       triNetSocketListen(listener, 10);

       triSocketSet masterSet;
       triSocketSet readSet;

       triNetSocketSetClear(&masterSet);
       triNetSocketSetClear(&readSet);

       triNetSocketSetAdd(listener, &masterSet);
       
       int maxSockets = listener;

       char helloToClient[] = "Welcome to my PSP!\n";

       char recvBuffer[1024];
       
       memset(recvBuffer, 0, sizeof(recvBuffer));
       
       int recvBytes;
       
       int handleNewSockets;
       
       int i, j;
       
       triInputInit();

       //HERE
       while(1)
       {
		while(1) {
		readSet = masterSet;

       		triInputUpdate();
			
		if(triInputPressed(PSP_CTRL_CROSS))
		{
			pspDebugScreenPrintf("You pressed CROSS\n");
			for(j = 0; j <= maxSockets; j++) {
                                if (j != listener) {
                                    triNetSocketSend(j, "The other PSP pressed CROSS", 27);
                                }
                        }
		}
		
		handleNewSockets = triNetSocketSelect(maxSockets+1, &readSet);
		
		if(handleNewSockets > 0)
			break;
		}

       		for(i = 0; i <= maxSockets;i++)
       		{
			if(triNetSocketSetIsMember(i, &readSet))
			{
				if(i == listener)
				{
					newSocket = triNetSocketAccept(listener);

					triNetSocketSetAdd(newSocket, &masterSet);
					
					if(newSocket > maxSockets)
					{
						maxSockets = newSocket;
					}

					pspDebugScreenPrintf("New connection %d\n", newSocket);

					triNetSocketSend(newSocket, helloToClient, strlen(helloToClient));
				}
				else
				{
					recvBytes = triNetSocketReceive(i, recvBuffer);
					
					if(recvBytes == 0)
					{
						pspDebugScreenPrintf("Socket %d closed\n", i);
						triNetSocketSetRemove(i, &masterSet);
						triNetSocketClose(i);
					}
					else
					{
						for(j = 0; j <= maxSockets; j++) {
							// send to everyone!
							if (triNetSocketSetIsMember(j, &masterSet)) {
								// except the listener & socket it came from
								if (j != listener && j != i) {
									triNetSocketSend(j, recvBuffer, strlen(recvBuffer));
									}
								}
							}
						pspDebugScreenPrintf("%.*s\n", recvBytes, recvBuffer);
						memset(recvBuffer, 0, sizeof(recvBuffer));
					}
				}
			}
		}
	}
	
	}
	else // isServer
	{
		triSocket mySocket = triNetSocketCreate();
		
		int maxSockets = mySocket;

		triSocketSet masterSet;
		
		triNetSocketSetClear(&masterSet);
		
		triNetSocketSetAdd(mySocket, &masterSet);
		
		triNetSocketConnect(mySocket, ip, 55555);
		
		triInputInit();

		char recvBuffer[1024];

		memset(recvBuffer, 0, sizeof(recvBuffer));
		
		int recvBytes;
		
		int handleNewSockets;
		
		while(1)
		{
			while(1)
			{

			triInputUpdate();
			
			if(triInputPressed(PSP_CTRL_CROSS))
			{
				triNetSocketSend(mySocket, "The other PSP pressed CROSS", 15);
				pspDebugScreenPrintf("You pressed CROSS\n");
			}
			
			handleNewSockets = triNetSocketSelect(maxSockets+1, &masterSet);
			
			if(handleNewSockets > 0)
				break;

			}

			if(triNetSocketSetIsMember(mySocket, &masterSet))
			{
				recvBytes = triNetSocketReceive(mySocket, recvBuffer);

					if(recvBytes <= 0)
					{
						pspDebugScreenPrintf("Socket closed\n");
						triNetSocketSetRemove(mySocket, &masterSet);
						triNetSocketClose(mySocket);
					}
					else
					{
						pspDebugScreenPrintf("%.*s\n", recvBytes, recvBuffer);
						memset(recvBuffer, 0, sizeof(recvBuffer));
					}

			}
		}
	}

		pspDebugScreenPrintf("Disconnecting\n");

       triNetDisconnect();
       
       triMemoryShutdown();

       sceKernelSleepThread();
	   
       return 0;
}
