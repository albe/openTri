#ifndef __GAMETYPES_H__
#define __GAMETYPES_H__

#include "../triTypes.h"
#include "../triModel.h"



typedef struct
{
	triChar*	name;
	
	triModel*	model;
	
	triS32		texture;
	triU32		color;
	
	triU32		equipment;		// Bitmask
} gameCar;


typedef struct
{
	triS32		score;
	triS32		highestSpeed;
} gameStats;


typedef struct
{
	triChar		name[24];
	
	gameCar		car;
	gameStats	stats;
} gamePlayer;



typedef struct
{
	triU32		direction;			// a direction flag for the road (vertical/horizontal, left/right turn)
	triU32		lanes;				// bitmask of useable lanes in this segment (holes in road/bridge)
} gameMapField;


typedef struct
{
	triModel*	model;
	triU32		numFields;
	gameMapField*	fieldTypes;		// numFields structs describing the fields
} gameMapTile;



extern		triModel	carModels[3];


#endif // __GAMETYPES_H__
