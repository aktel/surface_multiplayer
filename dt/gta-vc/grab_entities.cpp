/*
	Jak ozdoba na torcie
*/

#include <iunknown.h>
#include <vice_city.h>
#include <vice_city_online.h>

struct  CPool
{
    dword   objects;
    dword   flags;
    dword   size;
    dword   top;
};
 
auto    PedPool         = (CPool **)    0x97F2AC;
auto    VehiclePool     = (CPool **)    0xA0FDE4;

template <typename EntType, typename SurfaceEnt = void>
class CVCUnknown : public CUnknown
{
public:
    /* global surface vars */
    qboolean    IsCheak;
    qboolean    IgnorePool;

    EntType    *gameent;    // CVehicle, CPed etc.
    SurfaceEnt *surfaceent; // SaveRestore or Net Manager
};

template <typename EntType>
void GrabPool( CPool *pool )
{
    CGMSurfPool *gmsurface = &gmsurf->GetSurfPool<EntType>();
    gmsurface->edicts = Mem_Relloc( gmsurf->mempool, gmsurface->edicts, pool[0]->size * sizeof(edict_t*) );   // Cheak dynamic size

    EntType *ent = ( EntType *) pool[0]->objects;

    for (dword i=0;i<pool[0]->size;i++)
    {
        edict_t *edict = gmsurf->edicts[i];
        IUnknown *unk = edict->pvUnknown;

        if ( !edict->free )
        {
            if ( edict->pvPrivateData != edict->pvUnknown->gameent )
            {
                if ( ent == NULL )   // if not valid
                { unk->Distroy( edict ); edict = NULL }
                else unk->Reconnect( edict );     // Reconnect to ISaveRestore or INetClass subsystem                                                                                                   // edict = Game->ReallocEdict( edict );
            }

            (CVCUnknown*)unk->IsCheak = true;
        } 
        else 
        {
            edict = Game->AllocEdict( edict ); 
            unk->Connect( edict ); // Good Link vc_class to surface subsystem
        }
    }
}

void GrabProcess()
{
    if ( Game.gmTime == prevTime )  // Cache valid
        return;

    GrabPool<CAutomobile>( VehiclePool );
    GrabPool<CPlayerPed>( PedPool );

    prevTime = Game.gmTime;
}