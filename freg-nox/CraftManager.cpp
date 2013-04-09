#include "CraftManager.h"

CraftManager craft_manager;

bool CraftManager::Craft(
		const craft_recipe & recipe,
		craft_item & result)
{
	const ushort size=recipe.size();
	for (ushort i=0; i<recipes.size(); ++i) {
		if ( recipes.at(i)->size()!=size+1 )
			continue;
		ushort j=0;
		for ( ; j<size && recipes.at(i)->at(j)->num==recipe.at(j)->num &&
				recipes.at(i)->at(j)->kind==recipe.at(j)->kind &&
				recipes.at(i)->at(j)->sub==recipe.at(j)->sub; ++j);
		if ( j==size ) {
			result.num=recipes.at(i)->at(j)->num;
			result.kind=recipes.at(i)->at(j)->kind;
			result.sub=recipes.at(i)->at(j)->sub;
			return true;
		}
	}
	return false;
}
