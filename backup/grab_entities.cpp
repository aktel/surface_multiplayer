/*
	Jak ozdoba na torcie
*/

#include <vice_city.h>

struct  CPool
{
    dword   objects;
    dword   flags;
    dword   size;
    dword   top;
};
 
auto    PedPool         = (CPool **)    0x97F2AC;
auto    VehiclePool     = (CPool **)    0xA0FDE4;
 
void GrabProcess()
{
    // Realloc surface pool
    surface->entity[ENT_AUTOMOBILE] = Mem_Relloc( surface->mempool, surface->entity[ENT_AUTOMOBILE], VehiclePool[0]->size * sizeof(edict_t*) ); 
    surface->entity[ENT_PLAYERPED] = Mem_Relloc( surface->mempool, surface->entity[ENT_PLAYERPED], PedPool[0]->size * sizeof(edict_t*) ); 

    CAutomobile *Automobile = (CAutomobile *)VehiclePool[0]->objects;

    for (dword i=0;i<VehiclePool[0]->size;i++)
    {
        edict_t *edict = surface->entity[ENT_AUTOMOBILE];

        if ( !edict->free )
        {
            if ( edict->pvPrivateData != &surface->Automobiles[i] )
            {
                if ( &Automobile[i] == NULL )   // if not valid
                { CSurfAutomobile::Distroy( edict ); edict = NULL; }   // Освобождаем класс и вызываем диструктор вручную Game->FreeEdict(edict) == ( delete edict->void* <---- Memory Leak ( destructor ignored ) )
                else CSurfAutomobile::Reconnect( edict, &surface->Automobiles[i] );     // Reconnect to ISaveRestore and INetClass subsystem                                                                                                   // edict = Game->ReallocEdict( edict );
            }

            edict->delayfree = 1;       // Если класс потеряется из пула удаляем его. 
        } 
        else 
        {
            edict = Game->AllocEdict( edict );    // Alloc handle for ISaveRestore and INetClass subsystem
            CSurfAutomobile::Connect( edict, &surface->Automobiles[i] ); // Good Link vc_class to surface subsystem
        }

        surface->entity[ENT_AUTOMOBILE][i] = edict; // Записываем .
    }

    CPlayerPed *PlayerPed = (CPlayer *)PedPool[0]->objects;
    for (DWORD i=0;i<PedPool[0]->size;i++)
    {
        edict_t *edict = surface->entity[ENT_PLAYERPED];

        if ( !edict->free )
        {
            if ( edict->pvPrivateData != &surface->PlayerPeds[i] )
            {
                if ( &PlayerPed[i] == NULL )
                    edict = Game->FreeEdict( edict );
                else CSurfPlayerPed::Reconnect( edict, &surface->PlayerPeds[i] );                                               //edict = Game->ReallocEdict( edict );
            }

            edict->delayfree = 1; 
        } 
        else 
        {
            edict = Game->AllocEdict( edict );
            CSurfPlayerPed::Connect( edict, &surface->PlayerPeds[i] ); 
        }

        surface->entity[ENT_PLAYERPED][i] = edict;
    }

    // for ( edicts ) if ( !--edict->delayfree ) edict.Free();
    Game->ThinkDelayFree();
}