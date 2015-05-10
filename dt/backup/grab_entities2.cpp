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
 
template <typename EntType>
void GrabPool( CPool *pool )
{
    edict_t **gmsurfacepool = &gmsurf->GetEdictsGroup<EntType>();
    *gmsurfacepool = Mem_Relloc( gmsurf->mempool, *gmsurfacepool, pool[0]->size * sizeof(edict_t*) );   // Cheak dynamic size

    EntType *ent = ( EntType *) pool[0]->objects;

    for (dword i=0;i<pool[0]->size;i++)
    {
        edict_t **edict = &gmsurf->GetEdictByGameId<EntType>(i);
        CUnknown *unk = gmsurf->GetUnknownByGameId<EntType>(i);

        if ( !edict->free )
        {
            if ( edict->pvPrivateData != unk )
            {
                if ( unk == NULL )   // if not valid
                { CSurfBase<EntType>::Distroy( *edict ); edict = NULL; }   // Освобождаем класс и вызываем диструктор вручную Game->FreeEdict(edict) == ( delete edict->void* <---- Memory Leak ( destructor ignored ) )
                else CSurfBase<EntType>::Reconnect( *edict, unk );     // Reconnect to ISaveRestore and INetClass subsystem                                                                                                   // edict = Game->ReallocEdict( edict );
            }

            *edict->lifetime = 0.0f; // Через сколько умрёт обьект - сколько живёт если значение негативное. 
        } 
        else 
        {
            *edict = Game->AllocEdict( *edict );    // Alloc handle for ISaveRestore and INetClass subsystem
            CSurfBase<EntType>::Connect( *edict, unk ); // Good Link vc_class to surface subsystem
        }

        
    }
}

void GrabProcess()
{
    GrabPool<CAutomobile>( VehiclePool );
    GrabPool<CPlayerPed>( PedPool );
    // Realloc surface pool
    surface->entity[ENT_AUTOMOBILE] =  
    surface->entity[ENT_PLAYERPED] = Mem_Relloc( surface->mempool, surface->entity[ENT_PLAYERPED], PedPool[0]->size * sizeof(edict_t*) ); 

    CAutomobile *Automobile = (CAutomobile *)VehiclePool[0]->objects;

    

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