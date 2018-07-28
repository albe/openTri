
#include "../triTypes.h"
#include "../triModel.h"
#include "gameTypes.h"


triModel*	carModels[3];


triBool gameLoadModels()
{
	carModels[0] = triModelsLoadTrim( "data/cars.trim", &nModels );
	if (carModels[0]==0) return(0);
}

