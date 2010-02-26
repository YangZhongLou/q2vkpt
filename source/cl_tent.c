/*
Copyright (C) 1997-2001 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
// cl_tent.c -- client side temporary entities

#include "cl_local.h"

typedef struct {
    enum {
        ex_free,
        ex_explosion,
        ex_misc,
        ex_flash,
        ex_mflash,
        ex_poly,
        ex_poly2,
        ex_light
    } type;

    entity_t    ent;
    int         frames;
    float       light;
    vec3_t      lightcolor;
    float       start;
    int         baseframe;
} explosion_t;

#define MAX_EXPLOSIONS  32

static explosion_t cl_explosions[MAX_EXPLOSIONS];

typedef struct {
    int     entity;
    int     dest_entity;
    qhandle_t model;
    int     endtime;
    vec3_t  offset;
    vec3_t  start, end;
} beam_t;

#define MAX_BEAMS   32

static beam_t      cl_beams[MAX_BEAMS];

//PMM - added this for player-linked beams.
//Currently only used by the plasma beam
static beam_t      cl_playerbeams[MAX_BEAMS];

#define MAX_SUSTAINS        32

//ROGUE
static cl_sustain_t    cl_sustains[MAX_SUSTAINS];
//ROGUE

//PGM
extern void CL_TeleportParticles (vec3_t org);
//PGM

void CL_BlasterParticles (vec3_t org, vec3_t dir);
void CL_ExplosionParticles (vec3_t org);
void CL_BFGExplosionParticles (vec3_t org);
// RAFAEL
void CL_BlueBlasterParticles (vec3_t org, vec3_t dir);

qhandle_t   cl_sfx_ric1;
qhandle_t   cl_sfx_ric2;
qhandle_t   cl_sfx_ric3;
qhandle_t   cl_sfx_lashit;
qhandle_t   cl_sfx_spark5;
qhandle_t   cl_sfx_spark6;
qhandle_t   cl_sfx_spark7;
qhandle_t   cl_sfx_railg;
qhandle_t   cl_sfx_rockexp;
qhandle_t   cl_sfx_grenexp;
qhandle_t   cl_sfx_watrexp;
// RAFAEL
qhandle_t   cl_sfx_plasexp;
qhandle_t   cl_sfx_footsteps[4];

qhandle_t   cl_mod_explode;
qhandle_t   cl_mod_smoke;
qhandle_t   cl_mod_flash;
qhandle_t   cl_mod_parasite_segment;
qhandle_t   cl_mod_grapple_cable;
qhandle_t   cl_mod_parasite_tip;
qhandle_t   cl_mod_explo4;
qhandle_t   cl_mod_bfg_explo;
qhandle_t   cl_mod_powerscreen;
// RAFAEL
qhandle_t   cl_mod_plasmaexplo;

//ROGUE
qhandle_t   cl_sfx_lightning;
qhandle_t   cl_sfx_disrexp;
qhandle_t   cl_mod_lightning;
qhandle_t   cl_mod_heatbeam;
qhandle_t   cl_mod_monster_heatbeam;
qhandle_t   cl_mod_explo4_big;




//ROGUE
/*
=================
CL_RegisterTEntSounds
=================
*/
void CL_RegisterTEntSounds (void)
{
    int     i;
    char    name[MAX_QPATH];

    // PMM - version stuff
//  Com_Printf ("%s\n", ROGUE_VERSION_STRING);
    // PMM
    cl_sfx_ric1 = S_RegisterSound ("world/ric1.wav");
    cl_sfx_ric2 = S_RegisterSound ("world/ric2.wav");
    cl_sfx_ric3 = S_RegisterSound ("world/ric3.wav");
    cl_sfx_lashit = S_RegisterSound("weapons/lashit.wav");
    cl_sfx_spark5 = S_RegisterSound ("world/spark5.wav");
    cl_sfx_spark6 = S_RegisterSound ("world/spark6.wav");
    cl_sfx_spark7 = S_RegisterSound ("world/spark7.wav");
    cl_sfx_railg = S_RegisterSound ("weapons/railgf1a.wav");
    cl_sfx_rockexp = S_RegisterSound ("weapons/rocklx1a.wav");
    cl_sfx_grenexp = S_RegisterSound ("weapons/grenlx1a.wav");
    cl_sfx_watrexp = S_RegisterSound ("weapons/xpld_wat.wav");
    // RAFAEL
    // cl_sfx_plasexp = S_RegisterSound ("weapons/plasexpl.wav");
    S_RegisterSound ("player/land1.wav");

    S_RegisterSound ("player/fall2.wav");
    S_RegisterSound ("player/fall1.wav");

    for (i=0 ; i<4 ; i++)
    {
        Q_snprintf (name, sizeof(name), "player/step%i.wav", i+1);
        cl_sfx_footsteps[i] = S_RegisterSound (name);
    }

//PGM
    cl_sfx_lightning = S_RegisterSound ("weapons/tesla.wav");
    cl_sfx_disrexp = S_RegisterSound ("weapons/disrupthit.wav");
    // version stuff
//  sprintf (name, "weapons/sound%d.wav", ROGUE_VERSION_ID);
//  if (name[0] == 'w')
//      name[0] = 'W';
//PGM
}   

/*
=================
CL_RegisterTEntModels
=================
*/
void CL_RegisterTEntModels (void)
{
    cl_mod_explode = R_RegisterModel ("models/objects/explode/tris.md2");
    cl_mod_smoke = R_RegisterModel ("models/objects/smoke/tris.md2");
    cl_mod_flash = R_RegisterModel ("models/objects/flash/tris.md2");
    cl_mod_parasite_segment = R_RegisterModel ("models/monsters/parasite/segment/tris.md2");
    cl_mod_grapple_cable = R_RegisterModel ("models/ctf/segment/tris.md2");
    cl_mod_parasite_tip = R_RegisterModel ("models/monsters/parasite/tip/tris.md2");
    cl_mod_explo4 = R_RegisterModel ("models/objects/r_explode/tris.md2");
    cl_mod_bfg_explo = R_RegisterModel ("sprites/s_bfg2.sp2");
    cl_mod_powerscreen = R_RegisterModel ("models/items/armor/effect/tris.md2");

    R_RegisterModel ("models/objects/laser/tris.md2");
    R_RegisterModel ("models/objects/grenade2/tris.md2");
    R_RegisterModel ("models/weapons/v_machn/tris.md2");
    R_RegisterModel ("models/weapons/v_handgr/tris.md2");
    R_RegisterModel ("models/weapons/v_shotg2/tris.md2");
    R_RegisterModel ("models/objects/gibs/bone/tris.md2");
    R_RegisterModel ("models/objects/gibs/sm_meat/tris.md2");
    R_RegisterModel ("models/objects/gibs/bone2/tris.md2");
    // RAFAEL
    // R_RegisterModel ("models/objects/blaser/tris.md2");

    R_RegisterPic ("w_machinegun");
    R_RegisterPic ("a_bullets");
    R_RegisterPic ("i_health");
    R_RegisterPic ("a_grenades");

//ROGUE
    cl_mod_explo4_big = R_RegisterModel ("models/objects/r_explode2/tris.md2");
    cl_mod_lightning = R_RegisterModel ("models/proj/lightning/tris.md2");
    cl_mod_heatbeam = R_RegisterModel ("models/proj/beam/tris.md2");
    cl_mod_monster_heatbeam = R_RegisterModel ("models/proj/widowbeam/tris.md2");
//ROGUE
}   

/*
=================
CL_ClearTEnts
=================
*/
void CL_ClearTEnts (void)
{
    memset (cl_beams, 0, sizeof(cl_beams));
    memset (cl_explosions, 0, sizeof(cl_explosions));

//ROGUE
    memset (cl_playerbeams, 0, sizeof(cl_playerbeams));
    memset (cl_sustains, 0, sizeof(cl_sustains));
//ROGUE
}

static explosion_t *alloc_explosion( void ) {
    explosion_t *e, *oldest;
    int     i;
    int     time;
    
    for( i = 0, e = cl_explosions; i < MAX_EXPLOSIONS; i++, e++ ) {
        if (e->type == ex_free) {
            memset (e, 0, sizeof (*e));
            return e;
        }
    }
// find the oldest explosion
    time = cl.time;
    oldest = cl_explosions;

    for(i = 0, e = cl_explosions; i < MAX_EXPLOSIONS; i++, e++ ) {
        if ( e->start < time) {
            time = e->start;
            oldest = e;
        }
    }
    memset (oldest, 0, sizeof (*oldest));
    return oldest;
}

/*
=================
CL_SmokeAndFlash
=================
*/
void CL_SmokeAndFlash(vec3_t origin)
{
    explosion_t *ex;

    ex = alloc_explosion ();
    VectorCopy (origin, ex->ent.origin);
    ex->type = ex_misc;
    ex->frames = 4;
    ex->ent.flags = RF_TRANSLUCENT;
    ex->start = cl.servertime - cl.frametime;
    ex->ent.model = cl_mod_smoke;

    ex = alloc_explosion ();
    VectorCopy (origin, ex->ent.origin);
    ex->type = ex_flash;
    ex->ent.flags = RF_FULLBRIGHT;
    ex->frames = 2;
    ex->start = cl.servertime - cl.frametime;
    ex->ent.model = cl_mod_flash;
}

static void CL_ParseBeam (qhandle_t model)
{
    beam_t  *b;
    int     i;
    
// override any beam with the same source AND destination entities
    for (i=0, b=cl_beams ; i< MAX_BEAMS ; i++, b++)
        if (b->entity == te.entity1 && b->dest_entity == te.entity2)
            goto override;

// find a free beam
    for (i=0, b=cl_beams ; i< MAX_BEAMS ; i++, b++)
    {
        if (!b->model || b->endtime < cl.time)
        {
override:
            b->entity = te.entity1;
            b->dest_entity = te.entity2;
            b->model = model;
            b->endtime = cl.time + 200;
            VectorCopy (te.pos1, b->start);
            VectorCopy (te.pos2, b->end);
            VectorCopy (te.offset, b->offset);
            return;
        }
    }
}

// ROGUE
static void CL_ParsePlayerBeam (qhandle_t model)
{
    beam_t  *b;
    int     i;
    
// override any beam with the same entity
// PMM - For player beams, we only want one per player (entity) so..
    for (i=0, b=cl_playerbeams ; i< MAX_BEAMS ; i++, b++)
    {
        if (b->entity == te.entity1)
        {
            b->entity = te.entity1;
            b->model = model;
            b->endtime = cl.time + 200;
            VectorCopy (te.pos1, b->start);
            VectorCopy (te.pos2, b->end);
            VectorCopy (te.offset, b->offset);
            return;
        }
    }

// find a free beam
    for (i=0, b=cl_playerbeams ; i< MAX_BEAMS ; i++, b++)
    {
        if (!b->model || b->endtime < cl.time)
        {
            b->entity = te.entity1;
            b->model = model;
            b->endtime = cl.time + 100;     // PMM - this needs to be 100 to prevent multiple heatbeams
            VectorCopy (te.pos1, b->start);
            VectorCopy (te.pos2, b->end);
            VectorCopy (te.offset, b->offset);
            return;
        }
    }

}
//rogue

static cl_sustain_t *alloc_sustain( void ) {
    int     i;
    cl_sustain_t    *s;

    for (i=0, s=cl_sustains; i<MAX_SUSTAINS; i++, s++) {
        if (s->id == 0) {
            return s;
        }
    }
    return NULL;
}

//=============
//ROGUE
static void CL_ParseSteam (void) {
    cl_sustain_t    *s;

    if( te.entity1 == -1 ) {
        CL_ParticleSteamEffect (te.pos1, te.dir, te.color & 0xff, te.count, te.entity2);
        return;
    }
        
    s = alloc_sustain();
    if (!s)
        return;
    
    s->id = te.entity1;
    s->count = te.count;
    VectorCopy( te.pos1, s->org );
    VectorCopy( te.dir, s->dir );
    s->color = te.color & 0xff;
    s->magnitude = te.entity2;
    s->endtime = cl.time + te.time;
    s->think = CL_ParticleSteamEffect2;
    s->thinkinterval = 100;
    s->nextthink = cl.time;
}

static void CL_ParseWidow (void) {
    cl_sustain_t    *s;

    s = alloc_sustain();
    if (!s)
        return;

    s->id = te.entity1;
    VectorCopy (te.pos1, s->org);
    s->endtime = cl.time + 2100;
    s->think = CL_Widowbeamout;
    s->thinkinterval = 1;
    s->nextthink = cl.time;
}

static void CL_ParseNuke (void) {
    cl_sustain_t    *s;

    s = alloc_sustain();
    if (!s)
        return;

    s->id = 21000;
    VectorCopy (te.pos1, s->org);
    s->endtime = cl.time + 1000;
    s->think = CL_Nukeblast;
    s->thinkinterval = 1;
    s->nextthink = cl.time;
}

//ROGUE
//=============

static explosion_t *plain_explosion( void ) {
    explosion_t *ex;

    ex = alloc_explosion ();
    VectorCopy (te.pos1, ex->ent.origin);
    ex->type = ex_poly;
    ex->ent.flags = RF_FULLBRIGHT;
    ex->start = cl.servertime - cl.frametime;
    ex->light = 350;
    VectorSet( ex->lightcolor, 1.0, 0.5, 0.5 );
    ex->ent.angles[1] = rand() % 360;
    ex->ent.model = cl_mod_explo4;
    if (frand() < 0.5)
        ex->baseframe = 15;
    ex->frames = 15;

    return ex;
}

static void dirtoangles( vec3_t angles ) {
    angles[0] = acos(te.dir[2])/M_PI*180;
// PMM - fixed to correct for pitch of 0
    if (te.dir[0])
        angles[1] = atan2(te.dir[1], te.dir[0])/M_PI*180;
    else if (te.dir[1] > 0)
        angles[1] = 90;
    else if (te.dir[1] < 0)
        angles[1] = 270;
    else
        angles[1] = 0;
}

/*
=================
CL_AddTEnt
=================
*/
static const byte splash_color[] = {0x00, 0xe0, 0xb0, 0x50, 0xd0, 0xe0, 0xe8};

void CL_AddTEnt (void)
{
    explosion_t *ex;
    int r;

    switch (te.type)
    {
    case TE_BLOOD:          // bullet hitting flesh
        if( !( cl_disable_particles->integer & NOPART_BLOOD ) ) {
            CL_ParticleEffect (te.pos1, te.dir, 0xe8, 60);
        }
        break;

    case TE_GUNSHOT:            // bullet hitting wall
    case TE_SPARKS:
    case TE_BULLET_SPARKS:
        if (te.type == TE_GUNSHOT)
            CL_ParticleEffect (te.pos1, te.dir, 0, 40);
        else
            CL_ParticleEffect (te.pos1, te.dir, 0xe0, 6);

        if (te.type != TE_SPARKS) {
            CL_SmokeAndFlash(te.pos1);
            
            // impact sound
            r = rand()&15;
            if (r == 1)
                S_StartSound (te.pos1, 0, 0, cl_sfx_ric1, 1, ATTN_NORM, 0);
            else if (r == 2)
                S_StartSound (te.pos1, 0, 0, cl_sfx_ric2, 1, ATTN_NORM, 0);
            else if (r == 3)
                S_StartSound (te.pos1, 0, 0, cl_sfx_ric3, 1, ATTN_NORM, 0);
        }

        break;
        
    case TE_SCREEN_SPARKS:
    case TE_SHIELD_SPARKS:
        if (te.type == TE_SCREEN_SPARKS)
            CL_ParticleEffect (te.pos1, te.dir, 0xd0, 40);
        else
            CL_ParticleEffect (te.pos1, te.dir, 0xb0, 40);
        //FIXME : replace or remove this sound
        S_StartSound (te.pos1, 0, 0, cl_sfx_lashit, 1, ATTN_NORM, 0);
        break;
        
    case TE_SHOTGUN:            // bullet hitting wall
        CL_ParticleEffect (te.pos1, te.dir, 0, 20);
        CL_SmokeAndFlash(te.pos1);
        break;

    case TE_SPLASH:         // bullet hitting water
        if (te.color < 0 || te.color > 6)
            r = 0x00;
        else
            r = splash_color[te.color];
        CL_ParticleEffect (te.pos1, te.dir, r, te.count);

        if (te.color == SPLASH_SPARKS)
        {
            r = rand() & 3;
            if (r == 0)
                S_StartSound (te.pos1, 0, 0, cl_sfx_spark5, 1, ATTN_STATIC, 0);
            else if (r == 1)
                S_StartSound (te.pos1, 0, 0, cl_sfx_spark6, 1, ATTN_STATIC, 0);
            else
                S_StartSound (te.pos1, 0, 0, cl_sfx_spark7, 1, ATTN_STATIC, 0);
        }
        break;

    case TE_LASER_SPARKS:
        CL_ParticleEffect2 (te.pos1, te.dir, te.color, te.count);
        break;

    // RAFAEL
    case TE_BLUEHYPERBLASTER:
        CL_BlasterParticles (te.pos1, te.dir);
        break;

    case TE_BLASTER:            // blaster hitting wall
    case TE_BLASTER2:           // green blaster hitting wall
    case TE_FLECHETTE:          // flechette
        ex = alloc_explosion ();
        VectorCopy (te.pos1, ex->ent.origin);
        dirtoangles( ex->ent.angles );
        ex->type = ex_misc;
        ex->ent.flags = RF_FULLBRIGHT|RF_TRANSLUCENT;
        switch(te.type) {
        case TE_BLASTER:
            CL_BlasterParticles( te.pos1, te.dir );
            ex->lightcolor[0] = 1;
            ex->lightcolor[1] = 1;
            break;
        case TE_BLASTER2:
            CL_BlasterParticles2 (te.pos1, te.dir, 0xd0);
            ex->ent.skinnum = 1;
            ex->lightcolor[1] = 1;
            break;
        case TE_FLECHETTE:
            CL_BlasterParticles2 (te.pos1, te.dir, 0x6f); // 75
            ex->ent.skinnum = 2;
            ex->lightcolor[0] = 0.19;
            ex->lightcolor[1] = 0.41;
            ex->lightcolor[2] = 0.75;
            break;
        }
        ex->start = cl.servertime - cl.frametime;
        ex->light = 150;
        ex->ent.model = cl_mod_explode;
        ex->frames = 4;
        S_StartSound (te.pos1,  0, 0, cl_sfx_lashit, 1, ATTN_NORM, 0);
        break;
        
    case TE_RAILTRAIL:          // railgun effect
        CL_RailTrail (te.pos1, te.pos2);
        S_StartSound (te.pos2, 0, 0, cl_sfx_railg, 1, ATTN_NORM, 0);
        break;

    case TE_GRENADE_EXPLOSION:
    case TE_GRENADE_EXPLOSION_WATER:
        ex = plain_explosion ();
        ex->frames = 19;
        ex->baseframe = 30;

        if( cl_disable_explosions->integer & NOEXP_GRENADE ) {
            ex->type = ex_light;
        }
        
        if( !( cl_disable_particles->integer & NOPART_GRENADE_EXPLOSION ) ) {
            CL_ExplosionParticles( te.pos1 );
        }
        if (te.type == TE_GRENADE_EXPLOSION_WATER)
            S_StartSound (te.pos1, 0, 0, cl_sfx_watrexp, 1, ATTN_NORM, 0);
        else
            S_StartSound (te.pos1, 0, 0, cl_sfx_grenexp, 1, ATTN_NORM, 0);
        break;

    case TE_EXPLOSION2:
        ex = plain_explosion ();
        ex->frames = 19;
        ex->baseframe = 30;
        CL_ExplosionParticles( te.pos1 );
        S_StartSound (te.pos1, 0, 0, cl_sfx_grenexp, 1, ATTN_NORM, 0);
        break;


    // RAFAEL
    case TE_PLASMA_EXPLOSION:
        plain_explosion();
        CL_ExplosionParticles (te.pos1);
        S_StartSound (te.pos1, 0, 0, cl_sfx_rockexp, 1, ATTN_NORM, 0);
        break;

    case TE_ROCKET_EXPLOSION:
    case TE_ROCKET_EXPLOSION_WATER:
        ex = plain_explosion();
        if( cl_disable_explosions->integer & NOEXP_ROCKET ) {
            ex->type = ex_light;
        }
        if( !( cl_disable_particles->integer & NOPART_ROCKET_EXPLOSION ) ) {
            CL_ExplosionParticles( te.pos1 );
        }

        if( te.type == TE_ROCKET_EXPLOSION_WATER )
            S_StartSound( te.pos1, 0, 0, cl_sfx_watrexp, 1, ATTN_NORM, 0 );
        else
            S_StartSound( te.pos1, 0, 0, cl_sfx_rockexp, 1, ATTN_NORM, 0 );
        break;
    
    case TE_EXPLOSION1:
        plain_explosion();
        CL_ExplosionParticles( te.pos1 );
        S_StartSound( te.pos1, 0, 0, cl_sfx_rockexp, 1, ATTN_NORM, 0 );
        break;

    case TE_EXPLOSION1_NP:                      // PMM
        plain_explosion();
        S_StartSound( te.pos1, 0, 0, cl_sfx_rockexp, 1, ATTN_NORM, 0 );
        break;

    case TE_EXPLOSION1_BIG:                     // PMM
        ex = plain_explosion();
        ex->ent.model = cl_mod_explo4_big;
        S_StartSound( te.pos1, 0, 0, cl_sfx_rockexp, 1, ATTN_NORM, 0 );
        break;

    case TE_BFG_EXPLOSION:
        ex = alloc_explosion ();
        VectorCopy (te.pos1, ex->ent.origin);
        ex->type = ex_poly;
        ex->ent.flags = RF_FULLBRIGHT;
        ex->start = cl.servertime - cl.frametime;
        ex->light = 350;
        ex->lightcolor[0] = 0.0;
        ex->lightcolor[1] = 1.0;
        ex->lightcolor[2] = 0.0;
        ex->ent.model = cl_mod_bfg_explo;
        ex->ent.flags |= RF_TRANSLUCENT;
        ex->ent.alpha = 0.30;
        ex->frames = 4;
        break;

    case TE_BFG_BIGEXPLOSION:
        CL_BFGExplosionParticles (te.pos1);
        break;

    case TE_BFG_LASER:
        CL_ParseLaser (0xd0d1d2d3);
        break;

    case TE_BUBBLETRAIL:
        CL_BubbleTrail (te.pos1, te.pos2);
        break;

    case TE_PARASITE_ATTACK:
    case TE_MEDIC_CABLE_ATTACK:
        VectorClear( te.offset );
        te.entity2 = 0;
        CL_ParseBeam (cl_mod_parasite_segment);
        break;

    case TE_BOSSTPORT:          // boss teleporting to station
        CL_BigTeleportParticles (te.pos1);
        S_StartSound (te.pos1, 0, 0, S_RegisterSound ("misc/bigtele.wav"), 1, ATTN_NONE, 0);
        break;

    case TE_GRAPPLE_CABLE:
        te.entity2 = 0;
        CL_ParseBeam (cl_mod_grapple_cable);
        break;

    // RAFAEL
    case TE_WELDING_SPARKS:
        CL_ParticleEffect2 (te.pos1, te.dir, te.color, te.count);

        ex = alloc_explosion ();
        VectorCopy (te.pos1, ex->ent.origin);
        ex->type = ex_flash;
        // note to self
        // we need a better no draw flag
        ex->ent.flags = RF_BEAM;
        ex->start = cl.servertime - cl.frametime;
        ex->light = 100 + (rand()%75);
        ex->lightcolor[0] = 1.0;
        ex->lightcolor[1] = 1.0;
        ex->lightcolor[2] = 0.3;
        ex->ent.model = cl_mod_flash;
        ex->frames = 2;
        break;

    case TE_GREENBLOOD:
        CL_ParticleEffect2 (te.pos1, te.dir, 0xdf, 30);
        break;

    // RAFAEL
    case TE_TUNNEL_SPARKS:
        CL_ParticleEffect3 (te.pos1, te.dir, te.color, te.count);
        break;


    case TE_LIGHTNING:
        S_StartSound (NULL, te.entity1, CHAN_WEAPON, cl_sfx_lightning, 1, ATTN_NORM, 0);
        VectorClear( te.offset );
        CL_ParseBeam (cl_mod_lightning);
        break;

    case TE_DEBUGTRAIL:
        CL_DebugTrail (te.pos1, te.pos2);
        break;

    case TE_PLAIN_EXPLOSION:
        plain_explosion();
        break;

    case TE_FLASHLIGHT:
        CL_Flashlight(te.entity1, te.pos1);
        break;

    case TE_FORCEWALL:
        CL_ForceWall(te.pos1, te.pos2, te.color);
        break;

    case TE_HEATBEAM:
        VectorSet(te.offset, 2, 7, -3);
        CL_ParsePlayerBeam (cl_mod_heatbeam);
        break;

    case TE_MONSTER_HEATBEAM:
        VectorClear(te.offset);
        //CL_ParsePlayerBeam (cl_mod_monster_heatbeam);
        CL_ParsePlayerBeam (cl_mod_heatbeam); // FIXME?
        break;

    case TE_HEATBEAM_SPARKS:
        CL_ParticleSteamEffect (te.pos1, te.dir, 0x8, 50, 60);
        S_StartSound (te.pos1,  0, 0, cl_sfx_lashit, 1, ATTN_NORM, 0);
        break;
    
    case TE_HEATBEAM_STEAM:
        CL_ParticleSteamEffect (te.pos1, te.dir, 0xE0, 20, 60);
        S_StartSound (te.pos1,  0, 0, cl_sfx_lashit, 1, ATTN_NORM, 0);
        break;

    case TE_STEAM:
        CL_ParseSteam();
        break;

    case TE_BUBBLETRAIL2:
        CL_BubbleTrail2 (te.pos1, te.pos2, 8);
        S_StartSound (te.pos1,  0, 0, cl_sfx_lashit, 1, ATTN_NORM, 0);
        break;

    case TE_MOREBLOOD:
        CL_ParticleEffect (te.pos1, te.dir, 0xe8, 250);
        break;

    case TE_CHAINFIST_SMOKE:
        VectorSet( te.dir, 0, 0, 1 );
        CL_ParticleSmokeEffect (te.pos1, te.dir, 0, 20, 20);
        break;

    case TE_ELECTRIC_SPARKS:
        CL_ParticleEffect (te.pos1, te.dir, 0x75, 40);
        //FIXME : replace or remove this sound
        S_StartSound (te.pos1, 0, 0, cl_sfx_lashit, 1, ATTN_NORM, 0);
        break;

    case TE_TRACKER_EXPLOSION:
        CL_ColorFlash (te.pos1, 0, 150, -1, -1, -1);
        CL_ColorExplosionParticles (te.pos1, 0, 1);
        S_StartSound (te.pos1, 0, 0, cl_sfx_disrexp, 1, ATTN_NORM, 0);
        break;

    case TE_TELEPORT_EFFECT:
    case TE_DBALL_GOAL:
        CL_TeleportParticles (te.pos1);
        break;

    case TE_WIDOWBEAMOUT:
        CL_ParseWidow ();
        break;

    case TE_NUKEBLAST:
        CL_ParseNuke ();
        break;

    case TE_WIDOWSPLASH:
        CL_WidowSplash ();
        break;
//PGM
//==============

    default:
        Com_Error (ERR_DROP, "%s: bad type", __func__);
    }
}

/*
=================
CL_AddBeams
=================
*/
static void CL_AddBeams (void)
{
    int         i,j;
    beam_t      *b;
    vec3_t      dist, org;
    float       d;
    entity_t    ent;
    float       yaw, pitch;
    float       forward;
    float       len, steps;
    float       model_length;
    
// update beams
    for (i=0, b=cl_beams ; i< MAX_BEAMS ; i++, b++)
    {
        if (!b->model || b->endtime < cl.time)
            continue;

        // if coming from the player, update the start position
        if (b->entity == cl.frame.clientNum+1)  // entity 0 is the world
        {
            VectorCopy (cl.refdef.vieworg, b->start);
            b->start[2] -= 22;  // adjust for view height
        }
        VectorAdd (b->start, b->offset, org);

    // calculate pitch and yaw
        VectorSubtract (b->end, org, dist);

        if (dist[1] == 0 && dist[0] == 0)
        {
            yaw = 0;
            if (dist[2] > 0)
                pitch = 90;
            else
                pitch = 270;
        }
        else
        {
    // PMM - fixed to correct for pitch of 0
            if (dist[0])
                yaw = (atan2(dist[1], dist[0]) * 180 / M_PI);
            else if (dist[1] > 0)
                yaw = 90;
            else
                yaw = 270;
            if (yaw < 0)
                yaw += 360;
    
            forward = sqrt (dist[0]*dist[0] + dist[1]*dist[1]);
            pitch = (atan2(dist[2], forward) * -180.0 / M_PI);
            if (pitch < 0)
                pitch += 360.0;
        }

    // add new entities for the beams
        d = VectorNormalize(dist);

        memset (&ent, 0, sizeof(ent));
        if (b->model == cl_mod_lightning)
        {
            model_length = 35.0;
            d-= 20.0;  // correction so it doesn't end in middle of tesla
        }
        else
        {
            model_length = 30.0;
        }
        steps = ceil(d/model_length);
        len = (d-model_length)/(steps-1);

        // PMM - special case for lightning model .. if the real length is shorter than the model,
        // flip it around & draw it from the end to the start.  This prevents the model from going
        // through the tesla mine (instead it goes through the target)
        if ((b->model == cl_mod_lightning) && (d <= model_length))
        {
//          Com_Printf ("special case\n");
            VectorCopy (b->end, ent.origin);
            // offset to push beam outside of tesla model (negative because dist is from end to start
            // for this beam)
//          for (j=0 ; j<3 ; j++)
//              ent.origin[j] -= dist[j]*10.0;
            ent.model = b->model;
            ent.flags = RF_FULLBRIGHT;
            ent.angles[0] = pitch;
            ent.angles[1] = yaw;
            ent.angles[2] = rand()%360;
            V_AddEntity (&ent);         
            return;
        }
        while (d > 0)
        {
            VectorCopy (org, ent.origin);
            ent.model = b->model;
            if (b->model == cl_mod_lightning)
            {
                ent.flags = RF_FULLBRIGHT;
                ent.angles[0] = -pitch;
                ent.angles[1] = yaw + 180.0;
                ent.angles[2] = rand()%360;
            }
            else
            {
                ent.angles[0] = pitch;
                ent.angles[1] = yaw;
                ent.angles[2] = rand()%360;
            }
            
//          Com_Printf("B: %d -> %d\n", b->entity, b->dest_entity);
            V_AddEntity (&ent);

            for (j=0 ; j<3 ; j++)
                org[j] += dist[j]*len;
            d -= model_length;
        }
    }
}

extern cvar_t *hand;

/*
=================
ROGUE - draw player locked beams
CL_AddPlayerBeams
=================
*/
static void CL_AddPlayerBeams (void)
{
    int         i,j;
    beam_t      *b;
    vec3_t      dist, org;
    float       d;
    entity_t    ent;
    float       yaw, pitch;
    float       forward;
    float       len, steps;
    int         framenum = 0;
    float       model_length;
    
    float       hand_multiplier;
//  frame_t     *oldframe;
    player_state_t  *ps, *ops;

//PMM
    if (info_hand->integer == 2)
        hand_multiplier = 0;
    else if (info_hand->integer == 1)
        hand_multiplier = -1;
    else
        hand_multiplier = 1;
//PMM

// update beams
    for (i=0, b=cl_playerbeams ; i< MAX_BEAMS ; i++, b++)
    {
        vec3_t      f,r,u;
        if (!b->model || b->endtime < cl.time)
            continue;

        if(cl_mod_heatbeam && (b->model == cl_mod_heatbeam))
        {

            // if coming from the player, update the start position
            if (b->entity == cl.frame.clientNum+1)  // entity 0 is the world
            {   
                // set up gun position
                ps = &cl.frame.ps;
                ops = &cl.oldframe.ps;

                if( !cl.oldframe.valid || cl.oldframe.number != cl.frame.number - 1 )
                    ops = ps;       // previous frame was dropped or invalid

                for (j=0 ; j<3 ; j++)
                {
                    b->start[j] = cl.refdef.vieworg[j] + ops->gunoffset[j]
                        + cl.lerpfrac * (ps->gunoffset[j] - ops->gunoffset[j]);
                }
                VectorMA (b->start, (hand_multiplier * b->offset[0]), cl.v_right, org);
                VectorMA (     org, b->offset[1], cl.v_forward, org);
                VectorMA (     org, b->offset[2], cl.v_up, org);
                if (info_hand->integer == 2) {
                    VectorMA (org, -1, cl.v_up, org);
                }
                // FIXME - take these out when final
                VectorCopy (cl.v_right, r);
                VectorCopy (cl.v_forward, f);
                VectorCopy (cl.v_up, u);

            }
            else
                VectorCopy (b->start, org);
        }
        else
        {
            // if coming from the player, update the start position
            if (b->entity == cl.frame.clientNum+1)  // entity 0 is the world
            {
                VectorCopy (cl.refdef.vieworg, b->start);
                b->start[2] -= 22;  // adjust for view height
            }
            VectorAdd (b->start, b->offset, org);
        }

    // calculate pitch and yaw
        VectorSubtract (b->end, org, dist);

//PMM
        if(cl_mod_heatbeam && (b->model == cl_mod_heatbeam) && (b->entity == cl.frame.clientNum+1))
        {
            vec_t len;

            len = VectorLength (dist);
            VectorScale (f, len, dist);
            VectorMA (dist, (hand_multiplier * b->offset[0]), r, dist);
            VectorMA (dist, b->offset[1], f, dist);
            VectorMA (dist, b->offset[2], u, dist);
            if (info_hand->integer == 2) {
                VectorMA (org, -1, cl.v_up, org);
            }
        }
//PMM

        if (dist[1] == 0 && dist[0] == 0)
        {
            yaw = 0;
            if (dist[2] > 0)
                pitch = 90;
            else
                pitch = 270;
        }
        else
        {
    // PMM - fixed to correct for pitch of 0
            if (dist[0])
                yaw = (atan2(dist[1], dist[0]) * 180 / M_PI);
            else if (dist[1] > 0)
                yaw = 90;
            else
                yaw = 270;
            if (yaw < 0)
                yaw += 360;
    
            forward = sqrt (dist[0]*dist[0] + dist[1]*dist[1]);
            pitch = (atan2(dist[2], forward) * -180.0 / M_PI);
            if (pitch < 0)
                pitch += 360.0;
        }
        
        if (cl_mod_heatbeam && (b->model == cl_mod_heatbeam))
        {
            if (b->entity != cl.frame.clientNum+1)
            {
                framenum = 2;
//              Com_Printf ("Third person\n");
                ent.angles[0] = -pitch;
                ent.angles[1] = yaw + 180.0;
                ent.angles[2] = 0;
//              Com_Printf ("%f %f - %f %f %f\n", -pitch, yaw+180.0, b->offset[0], b->offset[1], b->offset[2]);
                AngleVectors(ent.angles, f, r, u);
                    
                // if it's a non-origin offset, it's a player, so use the hardcoded player offset
                if (!VectorCompare (b->offset, vec3_origin))
                {
                    VectorMA (org, -(b->offset[0])+1, r, org);
                    VectorMA (org, -(b->offset[1]), f, org);
                    VectorMA (org, -(b->offset[2])-10, u, org);
                }
                else
                {
                    // if it's a monster, do the particle effect
                    CL_MonsterPlasma_Shell(b->start);
                }
            }
            else
            {
                framenum = 1;
            }
        }

        // if it's the heatbeam, draw the particle effect
        if ((cl_mod_heatbeam && (b->model == cl_mod_heatbeam) && (b->entity == cl.frame.clientNum+1)))
        {
            CL_Heatbeam (org, dist);
        }

    // add new entities for the beams
        d = VectorNormalize(dist);

        memset (&ent, 0, sizeof(ent));
        if (b->model == cl_mod_heatbeam)
        {
            model_length = 32.0;
        }
        else if (b->model == cl_mod_lightning)
        {
            model_length = 35.0;
            d-= 20.0;  // correction so it doesn't end in middle of tesla
        }
        else
        {
            model_length = 30.0;
        }
        steps = ceil(d/model_length);
        len = (d-model_length)/(steps-1);

        // PMM - special case for lightning model .. if the real length is shorter than the model,
        // flip it around & draw it from the end to the start.  This prevents the model from going
        // through the tesla mine (instead it goes through the target)
        if ((b->model == cl_mod_lightning) && (d <= model_length))
        {
//          Com_Printf ("special case\n");
            VectorCopy (b->end, ent.origin);
            // offset to push beam outside of tesla model (negative because dist is from end to start
            // for this beam)
//          for (j=0 ; j<3 ; j++)
//              ent.origin[j] -= dist[j]*10.0;
            ent.model = b->model;
            ent.flags = RF_FULLBRIGHT;
            ent.angles[0] = pitch;
            ent.angles[1] = yaw;
            ent.angles[2] = rand()%360;
            V_AddEntity (&ent);         
            return;
        }
        while (d > 0)
        {
            VectorCopy (org, ent.origin);
            ent.model = b->model;
            if(cl_mod_heatbeam && (b->model == cl_mod_heatbeam))
            {
//              ent.flags = RF_FULLBRIGHT|RF_TRANSLUCENT;
//              ent.alpha = 0.3;
                ent.flags = RF_FULLBRIGHT;
                ent.angles[0] = -pitch;
                ent.angles[1] = yaw + 180.0;
                ent.angles[2] = (cl.time) % 360;
//              ent.angles[2] = rand()%360;
                ent.frame = framenum;
            }
            else if (b->model == cl_mod_lightning)
            {
                ent.flags = RF_FULLBRIGHT;
                ent.angles[0] = -pitch;
                ent.angles[1] = yaw + 180.0;
                ent.angles[2] = rand()%360;
            }
            else
            {
                ent.angles[0] = pitch;
                ent.angles[1] = yaw;
                ent.angles[2] = rand()%360;
            }
            
//          Com_Printf("B: %d -> %d\n", b->entity, b->dest_entity);
            V_AddEntity (&ent);

            for (j=0 ; j<3 ; j++)
                org[j] += dist[j]*len;
            d -= model_length;
        }
    }
}

/*
=================
CL_AddExplosions
=================
*/
static void CL_AddExplosions (void)
{
    entity_t    *ent;
    int         i;
    explosion_t *ex;
    float       frac;
    int         f;

    memset (&ent, 0, sizeof(ent));

    for (i=0, ex=cl_explosions ; i< MAX_EXPLOSIONS ; i++, ex++)
    {
        if (ex->type == ex_free)
            continue;
        frac = (cl.time - ex->start)/100.0;
        f = floor(frac);

        ent = &ex->ent;

        switch (ex->type) {
        case ex_mflash:
            if (f >= ex->frames-1)
                ex->type = ex_free;
            break;
        case ex_misc:
        case ex_light:
            if (f >= ex->frames-1) {
                ex->type = ex_free;
                break;
            }
            ent->alpha = 1.0 - frac/(ex->frames-1);
            break;
        case ex_flash:
            if (f >= 1) {
                ex->type = ex_free;
                break;
            }
            ent->alpha = 1.0;
            break;
        case ex_poly:
            if (f >= ex->frames-1) {
                ex->type = ex_free;
                break;
            }

            ent->alpha = (16.0 - (float)f)/16.0;

            if (f < 10) {
                ent->skinnum = (f>>1);
                if (ent->skinnum < 0)
                    ent->skinnum = 0;
            } else {
                ent->flags |= RF_TRANSLUCENT;
                if (f < 13)
                    ent->skinnum = 5;
                else
                    ent->skinnum = 6;
            }
            break;
        case ex_poly2:
            if (f >= ex->frames-1) {
                ex->type = ex_free;
                break;
            }

            ent->alpha = (5.0 - (float)f)/5.0;
            ent->skinnum = 0;
            ent->flags |= RF_TRANSLUCENT;
            break;
        default:
            break;
        }

        if (ex->type == ex_free)
            continue;
        if (ex->light) {
            V_AddLight (ent->origin, ex->light*ent->alpha,
                ex->lightcolor[0], ex->lightcolor[1], ex->lightcolor[2]);
        }

        if( ex->type != ex_light ) {
            VectorCopy (ent->origin, ent->oldorigin);

            if (f < 0)
                f = 0;
            ent->frame = ex->baseframe + f + 1;
            ent->oldframe = ex->baseframe + f;
            ent->backlerp = 1.0 - cl.lerpfrac;

            V_AddEntity (ent);
        }
    }
}

// PMM - CL_Sustains
static void CL_ProcessSustain (void)
{
    cl_sustain_t    *s;
    int             i;

    for (i=0, s=cl_sustains; i< MAX_SUSTAINS; i++, s++)
    {
        if (s->id)
        {
            if ((s->endtime >= cl.time) && (cl.time >= s->nextthink))
            {
//              Com_Printf ("think %d %d %d\n", cl.time, s->nextthink, s->thinkinterval);
                s->think (s);
            }
            else if (s->endtime < cl.time)
                s->id = 0;
        }
    }
}

/*
=================
CL_AddTEnts
=================
*/
void CL_AddTEnts (void)
{
    CL_AddBeams ();
    // PMM - draw plasma beams
    CL_AddPlayerBeams ();
    CL_AddExplosions ();
    // PMM - set up sustain
    CL_ProcessSustain();
}



