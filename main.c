/*
** Jo Sega Saturn Engine
** Copyright (c) 2012-2017, Johannes Fetz (johannesfetz@gmail.com)
** All rights reserved.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are met:
**     * Redistributions of source code must retain the above copyright
**       notice, this list of conditions and the following disclaimer.
**     * Redistributions in binary form must reproduce the above copyright
**       notice, this list of conditions and the following disclaimer in the
**       documentation and/or other materials provided with the distribution.
**     * Neither the name of the Johannes Fetz nor the
**       names of its contributors may be used to endorse or promote products
**       derived from this software without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
** ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
** DISCLAIMED. IN NO EVENT SHALL Johannes Fetz BE LIABLE FOR ANY
** DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
** (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
** LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
** ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
** SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/***************************************************************************************************\
** Special Note : The 3D part on Jo Engine is still in development. So, some glitch may occur ;)
**                Btw, texture mapping for triangle base mesh (not quads) is experimental.
\***************************************************************************************************/


#include <jo/jo.h>
#include "collision.h"
#include "objects.h"
#include "hamster.h"
//#include "texture.h"


//#define TRANSPARENT_COLOR_INDEX_IN_FONT_PALETTE     (2)
//#define	LINE_COLOR_TABLE (VDP2_VRAM_A0	+ 0x1f400)
//#define	COLOR_RAM_ADR (VDP2_COLRAM	+ 0x00600)

extern Sint8 SynchConst;
Sint32 framerate;

//extern	CDATA	*collison_data[];
#define WORK_RAM_LOW 0x00200000
player_params 		player;
static XPDATA *xpdata_[30];
static PDATA *pdata_LP_[30];
static CDATA *cdata_[30];
level_section		map_section[200];
enemy 				enemies[30];
powerup				powerups[30];
game_params 		game;

jo_pos3Df                   pos;
jo_rot3Df                   rot;
jo_palette                  image_pal;


GOURAUDTBL		gourRealMax[GOUR_REAL_MAX];
Uint8			vwork[GOUR_REAL_MAX];
Uint8 			enableRTG = 1;

//Controls
#define KEY_PRESS(id, key)  ((Smpc_Peripheral[id].data & key) == 0)
#define KEY_DOWN(id, key)   ((Smpc_Peripheral[id].push & key) == 0)

static jo_camera    cam;
Sint16					cam_pos_x = CAM_DEFAULT_X;
Sint16					cam_pos_y = CAM_DEFAULT_Y;
Sint16					cam_pos_z = CAM_DEFAULT_Z;
Sint16					cam_angle_x = 0;
Sint16					cam_angle_y = 0;
Sint16					cam_angle_z = 0;
Sint16					cam_target_x = 0;
Sint16					cam_target_y = 0;
Sint16					cam_target_z = 0;
static Uint8			cam_zoom_num = 0;
static Sint16			cam_zoom = 0;
Sint16					delta_x = 0;
float 				delta_y = 0.0f;
Sint16 				delta_z = 0;
Sint16					cam_adj_x;
Sint16					cam_adj_z;
Sint16					next_cam_x;
Sint16					next_cam_z;
Uint8					cam_number = 1;

static bool			show_debug = false;	
Uint16				model_total = 0;

float				gravity = 0.25f;//0.25
float				wgravity = 0.025f;
float				max_gravity = 8.0f;//8.0f
float				wmax_gravity = 1.0f;//1.0f

static bool			use_light = true;
static Sint16			section_dist;

static Sint16			lerp_count;
static jo_sound     step_sound;
static jo_sound     die_sound;
static jo_sound     ouch_sound;
static jo_sound     jump_sound;
static jo_sound     pup_sound;
static jo_sound     flap_sound;
static jo_sound		boing_sound;
static jo_sound		effect_sound;

static Uint16		enemy_total;
static Uint16		powerup_total;
static Sint16		cam_height;

Uint8					map_builder_delete_mode = false;
Uint8					map_builder_mode = 0;
Uint16					map_builder_model=0;
Uint16					map_builder_enemy=0;
Uint16					map_builder_powerup=0;
Uint16					gridsize = 32;
int					total_sections = 0;
Uint16 					object_number = 0;
float					object_scale = 1;
Uint16					object_pol_num = 0;
Uint16					object_last_texture = 0;
Uint16					object_last_pol_num = 0;
bool					object_show_poly = 0;
Uint16					anim_tex_num;
Uint16					anim_frame_counter;
//int 				total_map_sections = 0;
Uint16					gouraud_counter = 0;


//from XL2//
void ztClearText(void)
{
    Uint16 i;
    for (i=0; i<64; ++i)
        jo_nbg2_printf(0, i,"                                                                ");
}

//XL2//
/**Simple function to draw the sprites with Color Lookup tables**/
/*void render_CLUT_sprite(unsigned int id, int x, int y, int z)
{
    FIXED s_pos[XYZS];    s_pos[X]=toFIXED(x); s_pos[Y]=toFIXED(y); s_pos[Z]=toFIXED(z); s_pos[S]=65536;
    SPR_ATTR spr_attributes = SPR_ATTRIBUTE(id, LUTidx(id), No_Gouraud, CL16Look | ECdis | HSSon | CL_Trans,  sprNoflip  );
    slPutSprite(s_pos , &spr_attributes , 0) ;
}*/

void replace_texture(XPDATA * pol)
{
	Uint32 cnt, nbPt;
    nbPt = pol->nbPolygon;
	cnt = 0;
	
	for (cnt=0; cnt < nbPt; cnt++)
    {	
	pol->attbl[cnt].texno = anim_tex_num;
	pol->attbl[cnt].colno = LUTidx(anim_tex_num);
	}	
	
}

void ham_colour(int hamColour, int hamColour2)
{
	XPDATA * ham_model;
	ham_model=(XPDATA *)&xpdata_ham_head;
	Uint32 start_pol, end_pol, cnt;
    start_pol = 8;
	end_pol = 28;
	
	for (cnt=start_pol; cnt < end_pol; cnt++)
    {	
	ham_model->attbl[cnt].texno = hamColour;
	ham_model->attbl[cnt].colno = LUTidx(hamColour);
	}

	start_pol = 0;
	end_pol = 8;
	
	for (cnt=start_pol; cnt < end_pol; cnt++)
    {	
	ham_model->attbl[cnt].texno = hamColour2;
	ham_model->attbl[cnt].colno = LUTidx(hamColour2);
	}
	
	ham_model=(XPDATA *)&xpdata_ham_body;
	start_pol = 4;
	end_pol = 14;
	
	for (cnt=start_pol; cnt < end_pol; cnt++)
    {	
	ham_model->attbl[cnt].texno = hamColour;
	ham_model->attbl[cnt].colno = LUTidx(hamColour);
	}

	start_pol = 0;
	end_pol = 4;
	
	for (cnt=start_pol; cnt < end_pol; cnt++)
    {	
	ham_model->attbl[cnt].texno = hamColour2;
	ham_model->attbl[cnt].colno = LUTidx(hamColour2);
	}
	
}

void 		stop_sounds()
{
jo_audio_stop_sound(&step_sound);
jo_audio_stop_sound(&die_sound);
jo_audio_stop_sound(&ouch_sound);
jo_audio_stop_sound(&jump_sound);
jo_audio_stop_sound(&pup_sound);
jo_audio_stop_sound(&flap_sound);
jo_audio_stop_sound(&boing_sound);
jo_audio_stop_sound(&effect_sound);
	
}

void effect(int type, int effect_x, float effect_y, int effect_z)
{
	
	switch(type)
	{
	case 0: 	
				player.effect_type = 0;
				stop_sounds();
				jo_audio_play_sound_on_channel(&effect_sound, 0);
				player.effect_size = 0.0f;
				player.effect_x = effect_x;
				player.effect_y = effect_y;
				player.effect_z = effect_z;
				break;	

	case 1: 	
				player.effect_type = 1;
				
				player.effect_size = 0.0f;
				player.effect_x = effect_x;
				player.effect_y = effect_y;
				player.effect_z = effect_z;
				break;	
		
	}
	
	
}

void change_player_type(int player_type)
{
	effect(1,player.x,player.y,player.z);
	
	
	
	switch(player_type)
	{
		
	case 0: player.type = 0;
			ham_colour(0,1);
			player.jump_height = -8.0f;
			player.rleg_rz = 0;
			player.lleg_rz = 0;
			player.xsize = 16;
			player.ysize = 16;
			player.ysize = 16;
			
			break;
			
	case 1: player.type = 1;
			ham_colour(ENEMY_TILESET,ENEMY_TILESET+2);
			player.jump_height = -2.6f;
			player.rleg_rz = 0;
			player.lleg_rz = 0;
			player.xsize = 8;
			player.ysize = 8;
			player.zsize = 8;
			break;
			
	case 2: player.type = 2;
			ham_colour(ENEMY_TILESET+2,ENEMY_TILESET+3);
			player.jump_height = -8.0f;
			player.rleg_rz = 0;
			player.lleg_rz = 0;
			player.xsize = 16;
			player.ysize = 16;
			player.ysize = 16;
			break;
			
	case 3: player.type = 3;
			ham_colour(ENEMY_TILESET+9,ENEMY_TILESET+10);
			player.jump_height = -16.0f;
			player.rleg_rz = -120;
			player.lleg_rz = 128;
			player.xsize = 16;
			player.ysize = 16;
			player.ysize = 16;
			break;
			
	case 4: player.type = 4;
			ham_colour(ENEMY_TILESET+11,ENEMY_TILESET+11);
			player.jump_height = -8.0f;
			player.xsize = 16;
			player.ysize = 16;
			player.zsize = 16;
			break;
			
	default:player.type = 0;
			ham_colour(0,1);
			player.jump_height = -6.0f;
			player.rleg_rz = 0;
			player.lleg_rz = 0;
			player.xsize = 16;
			player.ysize = 16;
			player.ysize = 16;
			break;

		
	}
	
	player.y = player.y-player.ysize;

	
}

/* 8 tiles */
static const jo_tile    HAM_Tileset[] =
{
	{0, 0, 16, 16},
	{0, 16, 16, 16},
	{16, 0, 8, 1},
	{16, 16, 16, 16},
	{16, 1, 8, 1},
	{32, 16, 16, 16},
	{48, 0, 16, 16},
	{48, 16, 16, 16},
	{32, 0, 8, 8}
	
};

/* 13 tiles */
static const jo_tile    ENEMY_Tileset[] =
{
	{0, 0, 16, 16},
	{56, 0, 8, 1},
	{16, 0, 16, 16},
	{16, 16, 16, 16},
	{56, 1, 8, 1},
	{48, 0, 8, 8},
	{32, 16, 16, 16},	
	{48, 8, 16, 24},
	{64, 8, 16, 24},
	{80, 0, 16, 16},
	{80, 16, 16, 16},
	{56, 2, 8, 1},
	{32, 0, 16, 16},
	{96, 8, 16, 24}
	//{0, 16, 8, 1},
};

static const jo_tile    MAP_Tileset[] =
{
	{0, 0, 48, 48},
	{0, 48, 48, 48},
	{192, 128, 32, 32},
	{0, 144, 48, 48},
	{0, 192, 48, 48},
	{48, 0, 48, 48},
	{48, 48, 48, 48},
	{48, 96, 48, 48},//arrow
	{48, 144, 48, 16},//gend
	{48, 192, 48, 24},//pet1
	{96, 0, 48, 48},
	{96, 48, 48, 48},
	{96, 96, 48, 48},
	{48, 160, 48, 16},//detail2
	{96, 144, 48, 48},
	{96, 192, 48, 48},//wfall
	{144, 0, 48, 48},
	{144, 48, 48, 48},
	{144, 96, 48, 48},
	{144, 144, 32, 32},//
	{144, 176, 32, 32},
	{144, 208, 32, 32},
	{192, 0, 32, 32},
	{192, 32, 32, 32},
	{192, 64, 32, 32},
	{192, 96, 32, 32},
	{176, 144, 16, 16},
	{176, 160, 48, 16},
	{176, 176, 48, 16},
	{176, 192, 48, 16},
	{48, 216, 48, 24},
	{176, 208, 8, 1},
	{176, 224, 8, 1},
	{192, 208, 32, 32},
	{0, 96, 48, 48}//rbed
	
	
};

int roundUp(Sint16 numToRound, Sint16 multiple)
{
    if (multiple == 0)
        return numToRound;

    int remainder = JO_ABS(numToRound) % multiple;
    if (remainder == 0)
        return numToRound;

    if (numToRound < 0)
        return -(JO_ABS(numToRound) - remainder);
    else
        return numToRound + multiple - remainder;
}

void animate_texture(Uint16 start_tex, Uint8 total_frames, Uint8 speed)
{
if (game.game_state != GAMESTATE_GAMEPLAY)
       return;
   if(anim_tex_num == 0)
   {
	   anim_tex_num = start_tex;
   }
   //30 fps?
   if(anim_frame_counter >= speed)
   {
	   anim_frame_counter = 0;
	   if(anim_tex_num == start_tex+total_frames)
	   {
			anim_tex_num = start_tex;
	   }else
	   {
	   anim_tex_num ++;
	   }
   }else
   {
	   anim_frame_counter ++;
   }
   
   
  
	
}

void apply_player_gravity(void)
{
	if(player.in_water)
	{
		if (player.speed_y < wmax_gravity)
		player.speed_y += (wgravity * framerate);
	}else
	{
		if (player.speed_y < max_gravity)
		player.speed_y += (gravity * framerate);
	}
	
}


void reset_demo(void)
{

cam_pos_x = CAM_DEFAULT_X;
cam_pos_y = CAM_DEFAULT_Y;
cam_height = CAM_DEFAULT_Y;
cam_pos_z = CAM_DEFAULT_Z;
cam_angle_x = 0;
cam_angle_y = 0;
cam_angle_z = 0;
cam_target_x = 0;
cam_target_y = 0;
cam_target_z = 0;
cam_zoom_num = 0;
cam_zoom = 0;
gravity = gravity;
wgravity = wgravity;

change_player_type(0);
player.x = WORLD_DEFAULT_X;
player.y = WORLD_DEFAULT_Y;
player.z = WORLD_DEFAULT_Z;
player.can_be_hurt = true;
player.shadow_y = player.y;
player.shadow_size = 1.0f;
player.left_cloud_size =1.0f;
player.effect_size = 2.0f;

player.speed_y = 0;
player.health = 3;
player.mutate = false;
apply_player_gravity();

for(Uint16 e = 0; e < enemy_total; e++)
	{
		enemies[e].alive = true;
		enemies[e].x = enemies[e].start_x;
		enemies[e].y = enemies[e].start_y;
		enemies[e].z = enemies[e].start_z;
		enemies[e].jump_timer = ENEMY_JUMP_TIMER;
		enemies[e].health = enemies[e].start_health;
		enemies[e].rz = 0;
	}

}



float Lerp(float a, float b,float t)
{
	lerp_count++;
	return a + t * (b - a);
	
}



void				player_jump(void)
{
			player.state = jumping;
			stop_sounds();
			if(player.type == 3)
			{
			jo_audio_play_sound_on_channel(&boing_sound, 0);
			}else
			{
			jo_audio_play_sound_on_channel(&jump_sound, 0);
			}
			if(player.in_water)
			{
			player.speed_y = player.jump_height/3;
			}else
			{
			player.speed_y = player.jump_height;
			}
		
}



void				player_flap(void)
{
			player.flapping = true;
			stop_sounds();
			jo_audio_play_sound_on_channel(&flap_sound, 0);
			player.speed_y = player.jump_height;
		
}

void				player_bounce(void)
{
			stop_sounds();
			jo_audio_play_sound_on_channel(&die_sound, 3);
			player.speed_y = -8.0f;
		
}

void				player_bounceback(void)
{
			stop_sounds();
			jo_audio_play_sound_on_channel(&ouch_sound, 4);
			player.speed_y = -8.0f;
			
		
}

void pup_timer_counter(void)
{
	player.pup_timer += 1*framerate;
	if(player.pup_timer >= PUP_TIMER)
	{
	player.mutate = false;
	change_player_type(0);
	player.pup_timer = 0;
	}
	
	
}

void player_hurt(void)
{
	player.hurt_timer += 1*framerate;
			
			if(player.hurt_timer >= PLAYER_HURT_TIMER)
			{
				player.can_be_hurt = true;
				player.hurt_timer = 0;
				
			}
			if(player.health <= 0)
			{
			//player.alive = false;
			reset_demo();
			}

}


//map_builder Collision Detection

bool has_map_collision(level_section* map, Uint16 new_section_number, Sint16 new_section_x, Sint16 new_section_y, Sint16 new_section_z)
{
	bool x_collide = false;
	bool y_collide = false;
	bool z_collide = false;
	Sint16 x_dist;
	Sint16 y_dist;
	Sint16 z_dist;
	Sint16 collpoints_x;
	Sint16 collpoints_y;
	Sint16 collpoints_z;
	Uint16 collpoints_xsize;
	Uint16 collpoints_ysize;
	Uint16 collpoints_zsize;
	Uint8 collpoints_type;
	Uint16 collpoints_total;
	Uint16 ns_collpoints_total;
	Sint16 object_x;
	Sint16 object_y;
	Sint16 object_z;
	Uint16 object_xsize;
	Uint16 object_ysize;
	Uint16 object_zsize;
	CDATA		*ns_cdata;
	COLLISON	*ns_collison;
	
	
	ns_cdata=(CDATA *)cdata_[new_section_number];//(CDATA *)collison_data[new_section_number];
	ns_collison	=(COLLISON *)ns_cdata->cotbl;
	
	collpoints_total = map->a_cdata->nbCo;
	ns_collpoints_total = 	ns_cdata->nbCo;
	
	if(section_dist < COLL_DIST)
	{
		
		for(int j = 0; j < ns_collpoints_total; j++)
		{
			
			object_x = ns_collison[j].cen_x + new_section_x;
			object_y = ns_collison[j].cen_y + new_section_y;
			object_z = ns_collison[j].cen_z + new_section_z;
			
			object_xsize = ns_collison[j].x_size;
			object_ysize = ns_collison[j].y_size;
			object_zsize = ns_collison[j].z_size;

			for(int i = 0; i < collpoints_total; i++)
			{
				//	Reset values
				x_collide = false;
				y_collide = false;
				z_collide = false;
				
				
				collpoints_x = map->a_collison[i].cen_x + map->x + map->tx;
				collpoints_y = map->a_collison[i].cen_y + map->y + map->ty;
				collpoints_z = map->a_collison[i].cen_z + map->z + map->tz;
							
				collpoints_xsize = map->a_collison[i].x_size;
				collpoints_ysize = map->a_collison[i].y_size;
				collpoints_zsize = map->a_collison[i].z_size;
				collpoints_type = map->a_collison[i].att;
				
						 
				x_dist = JO_ABS(object_x - collpoints_x );
				y_dist = JO_ABS(object_y - collpoints_y );
				z_dist = JO_ABS(object_z - collpoints_z );
				
				
				//	Check x
				if(x_dist - (object_xsize + collpoints_xsize-8) <=0)
				{
					x_collide = true;
				}
				
				//	Check y
				if(y_dist - (object_ysize + collpoints_ysize-8) <=0)
				{
					y_collide = true;
				}
				
				//	Check z
				if(z_dist - (object_zsize + collpoints_zsize-8) <=0)
				{
					z_collide = true;
				}
					
				
				if(x_collide && y_collide && z_collide && collpoints_type == 0)
				{
					return true;
					break;
				}
			}
		}
	}
	return false;
}


//	check X or Z collision
bool has_horizontal_collision(Uint8 collpoints_type, Sint16 collpoints_x, Sint16 collpoints_y, Sint16 collpoints_z, Uint16 collpoints_xsize, Uint16 collpoints_ysize, Uint16 collpoints_zsize, Sint16 object_x, Sint16 object_y, Sint16 object_z, Uint16 object_xsize, Uint16 object_ysize, Uint16 object_zsize)
{
	bool x_collide = false;
	bool y_collide = false;
	bool z_collide = false;
	Uint16 x_dist;
	Uint16 y_dist;
	Uint16 z_dist;
	
			//	Reset values
			x_collide = false;
			y_collide = false;
			z_collide = false;
					 
			x_dist = JO_ABS(object_x - collpoints_x);
			y_dist = JO_ABS(object_y - collpoints_y);
			z_dist = JO_ABS(object_z - collpoints_z);
						
			//	Check x
			if(x_dist - (object_xsize + collpoints_xsize) <=0)
			{
				x_collide = true;
			}
			
			//	Check y
			if(y_dist - (object_ysize + collpoints_ysize) <=0)
			{
				y_collide = true;
			}
			
			//	Check z
			if(z_dist - (object_zsize + collpoints_zsize) <=0)
			{
				z_collide = true;
			}
			/*if(show_debug)
			{
			jo_nbg2_printf(0, 3, "ZCOL:\t%3d\t%3d\t%3d ",(int) x_collide, (int) y_collide, (int) z_collide);
			jo_nbg2_printf(0, 4, "ZDIS:\t%3d\t%3d\t%3d ",(int) x_dist, (int) y_dist, (int) z_dist);
			}*/
			//jo_nbg2_printf(0, 5, "Collisions:\t%d\t%d\t%d ",(int) x_collide, (int) y_collide, (int) z_collide);
			//jo_nbg2_printf(0, 6, "distance:\t%d\t%d\t%d ",(int) x_dist, (int) y_dist, (int) z_dist);
			
			//	Return if colliding
			
			
			if(x_collide && y_collide && z_collide && collpoints_type == 0)
			{
				return true;
			}
		
	return false;
}

float has_vertical_collision(Uint8 collpoints_type, Sint16 collpoints_x, Sint16 collpoints_y, Sint16 collpoints_z, Uint16 collpoints_xsize, Uint16 collpoints_ysize, Uint16 collpoints_zsize, float object_int_height, Sint16 object_x, Sint16 object_y, Sint16 object_z, Uint16 object_xsize, Uint16 object_ysize, Uint16 object_zsize)
{
	bool x_collide = false;
	bool y_collide = false;
	bool z_collide = false;
	Uint16 x_dist;
	Uint16 y_dist;
	Uint16 z_dist;
	Sint16 ramp_xdist;
	Sint16 ramp_zdist;
				
			//	Reset values
			x_collide = false;
			y_collide = false;
			z_collide = false;
				
			x_dist = JO_ABS(object_x - collpoints_x);
			y_dist = JO_ABS((object_y + object_ysize) - collpoints_y);
			z_dist = JO_ABS(object_z - collpoints_z);
			ramp_xdist = (collpoints_x - object_x);
			ramp_zdist = (collpoints_z - object_z);
						
			//	Check x
			if(x_dist - (object_xsize + collpoints_xsize) <=0)
			{
				x_collide = true;
			}
			
			//	Check y
			if(y_dist - (object_ysize + collpoints_ysize) <=0)
			{				
				y_collide = true;
			}
			
			//	Check z
			if(z_dist - (object_zsize + collpoints_zsize) <=0)
			{
				z_collide = true;
			}
			/*if(show_debug)
			{
			jo_nbg2_printf(0, 5, "VCOL:\t%3d\t%3d\t%3d ",(int) x_collide, (int) y_collide, (int) z_collide);
			jo_nbg2_printf(0, 6, "VDIS:\t%3d\t%3d\t%3d ",(int) x_dist, (int) y_dist, (int) z_dist);
			}*/
			//	Return if colliding
			if(x_collide && y_collide && z_collide)//&& collpoints_type == 0
			{
				
					//check for ramp
					if(collpoints_type == 0)
					{
						object_int_height = collpoints_y - (collpoints_ysize + JO_MULT_BY_2(object_ysize));
						/*if(show_debug)
						{
						jo_nbg2_printf(0, 8, "collpoints_y:       %3d ",collpoints_y);
						jo_nbg2_printf(0, 9, "collpoints_ysize:   %3d ",collpoints_ysize);
						jo_nbg2_printf(0, 10, "object_int_height: %3d ",object_int_height);
						
						}*/
					}
					else
					{
						//check for water
						if(collpoints_type == 9)
						{
						return NO_RAMP_COLLISION;	
						}
						
					float bottom_height = 0;
					float top_height = (int) collpoints_ysize + object_ysize;
					float location = 0;
						
					if(collpoints_type == 2)
					{
					location = (1-((float)(ramp_xdist- object_xsize)/(float)collpoints_xsize))/2;
					}
					if(collpoints_type == 4)
					{
					location = (1+((float)(ramp_xdist + object_xsize)/(float)collpoints_xsize))/2;
					}
					if(collpoints_type == 1)
					{
					location = (1-((float)(ramp_zdist - object_zsize)/(float)collpoints_zsize))/2;
					}
					if(collpoints_type == 3)
					{
					location = (1+((float)(ramp_zdist + object_zsize)/(float)collpoints_zsize))/2;
					}
					
					
					object_int_height = Lerp(bottom_height,top_height,location);//(height at bottom, height at top, location between bottom and top (between 0 and 1)) - remember this is reversed 0 is higher on screen than 32
					if(y_dist >(int) object_int_height)
					{
						/*if(show_debug)
						{
						jo_nbg2_printf(20, 1, "V object_int_height:%3d",(int) object_int_height );
						//jo_nbg2_printf(20, 7, "lerp:\t%3d\t%3d\t%3d ",(int) bottom_height,(int) top_height, (int)location * 100  );
						}*/
					return NO_RAMP_COLLISION;
					}
					
					}
				return object_int_height == NO_RAMP_COLLISION ? 1 : object_int_height;
			}
			
	return NO_RAMP_COLLISION;
	
}

float has_shadow_collision(Sint16 collpoints_x, Sint16 collpoints_y, Sint16 collpoints_z, Uint16 collpoints_xsize, Uint16 collpoints_ysize, Uint16 collpoints_zsize, Sint16 object_x, Sint16 object_y, Sint16 object_z, Uint16 object_xsize, Uint16 object_ysize, Uint16 object_zsize)
{
	bool x_collide = false;
	bool y_collide = false;
	bool z_collide = false;
	Uint16 x_dist;
	Uint16 y_dist;
	Uint16 z_dist;
			
			//	Reset values
			x_collide = false;
			y_collide = false;
			z_collide = false;
				
			x_dist = JO_ABS(object_x - collpoints_x);
			y_dist = JO_ABS(object_y - collpoints_y);
			z_dist = JO_ABS(object_z - collpoints_z);
						
			//	Check x
			if(x_dist - (object_xsize + collpoints_xsize) <=0)
			{
				x_collide = true;
			}
			
			//	Check y
			if(y_dist - (object_ysize + object_ysize + collpoints_ysize + collpoints_ysize) <=0)
			{				
				y_collide = true;
			}
			
			//	Check z
			if(z_dist - (object_zsize + collpoints_zsize) <=0)
			{
				z_collide = true;
			}
			/*if(show_debug)
			{
			jo_nbg2_printf(0, 5, "VCOL:\t%3d\t%3d\t%3d ",(int) x_collide, (int) y_collide, (int) z_collide);
			jo_nbg2_printf(0, 6, "VDIS:\t%3d\t%3d\t%3d ",(int) x_dist, (int) y_dist, (int) z_dist);
			}*/
			//	Return if colliding
			if(x_collide && y_collide && z_collide)//&& collpoints_type == 0
			{
				
				return y_dist - collpoints_ysize;
			}

	return NO_RAMP_COLLISION;
	
}

float has_ceiling_collision(Sint16 collpoints_x, Sint16 collpoints_y, Sint16 collpoints_z, Uint16 collpoints_xsize, Uint16 collpoints_ysize, Uint16 collpoints_zsize, Sint16 object_x, Sint16 object_y, Sint16 object_z, Uint16 object_xsize, Uint16 object_ysize, Uint16 object_zsize, float object_speed_y)
{
	
	bool x_collide = false;
	bool y_collide = false;
	bool z_collide = false;
	Uint16 x_dist;
	Uint16 y_dist;
	Uint16 z_dist;
	
			//	Reset values
			x_collide = false;
			y_collide = false;
			z_collide = false;
			
			x_dist = JO_ABS(object_x - collpoints_x);
			y_dist = JO_ABS((object_y - object_ysize) - collpoints_y);
			z_dist = JO_ABS(object_z - collpoints_z);
						
			//	Check x
			if(x_dist - (object_xsize + collpoints_xsize) <=0)
			{
				x_collide = true;
			}
			
			//	Check y
			if(y_dist - (object_ysize + collpoints_ysize) <=0)
			{				
				y_collide = true;
			}
			
			//	Check z
			if(z_dist - (object_zsize + collpoints_zsize) <=0)
			{
				z_collide = true;
			}
			/*if(show_debug)
			{
			jo_nbg2_printf(0, 5, "VCOL:\t%3d\t%3d\t%3d ",(int) x_collide, (int) y_collide, (int) z_collide);
			jo_nbg2_printf(0, 6, "VDIS:\t%3d\t%3d\t%3d ",(int) x_dist, (int) y_dist, (int) z_dist);
			}*/
			//	Return if colliding
			if(x_collide && y_collide && z_collide && object_speed_y <=0)
			{
			return true;
			}
			
	return false;
	
}

bool has_object_collision(Sint16 object1_x, Sint16 object1_y, Sint16 object1_z, Uint16 object1_xsize, Uint16 object1_ysize, Uint16 object1_zsize, Sint16 object1_speed_x,Sint16 object1_speed_z,
							Sint16 object2_x, Sint16 object2_y, Sint16 object2_z, Uint16 object2_xsize, Uint16 object2_ysize, Uint16 object2_zsize)
{
	bool x_collide = false;
	bool y_collide = false;
	bool z_collide = false;
	Uint16 x_dist;
	Uint16 y_dist;
	Uint16 z_dist;
	Sint16 next_pixel_x;
	Sint16 next_pixel_z;
	
	//set distance
	x_dist = JO_ABS(object1_x - object2_x);
	y_dist = JO_ABS(object1_y - object2_y);
	z_dist = JO_ABS(object1_z - object2_z);
	
	if((x_dist + y_dist + z_dist) < 250)
	{	
		//	Reset values
		x_collide = false;
		y_collide = false;
		z_collide = false;
		
				
		next_pixel_x = object1_speed_x > 0 ? object1_x + 4 :
                 object1_speed_x < 0 ? object1_x - 4 :
                 object1_x;
		next_pixel_z = object1_speed_z > 0 ? object1_z + 4 :
                 object1_speed_z < 0 ? object1_z - 4 :
                 object1_z;
		
				
		x_dist = JO_ABS(object2_x - next_pixel_x);
		y_dist = JO_ABS(object2_y - object1_y);
		z_dist = JO_ABS(object2_z - next_pixel_z);
		
		
		//	Check x
		if(x_dist - (object1_xsize + object2_xsize) <=0)
		{
			x_collide = true;
		}
		
		//	Check y
		if(y_dist - (object1_ysize + object2_ysize) <=0)
		{
			y_collide = true;
		}
		
		//	Check z
		if(z_dist - (object1_zsize + object2_zsize) <=0)
		{
			z_collide = true;
		}
		
		//if(show_debug)
	//	{
		//jo_nbg2_printf(0, 1, "OBJECT COL:\t%3d\t%3d\t%3d ",(int) x_collide, (int) y_collide, (int) z_collide);
		
		//}
		
		//	Return if colliding
		if(x_collide && y_collide && z_collide)
		{
			return true;
		}
		
	
	return false;
	}
	else
		return false;
	
}




void player_collision_handling(void)
{
	float collide;
	bool ycollide = 0;
	bool xcollide = 0;
	bool zcollide = 0;
	bool ccollide = 0;
	bool wcollide = 0;
	Uint16 x_dist;
	Uint16 y_dist;
	Uint16 z_dist;
	Uint16 collpoints_total;
	Sint16 collpoints_x;
	Sint16 collpoints_y;
	Sint16 collpoints_z;
	Uint16 collpoints_xsize;
	Uint16 collpoints_ysize;
	Uint16 collpoints_zsize;
	Uint8 collpoints_type;
	
	if(!player.can_be_hurt)
		{
		player_hurt();
		}
		
	player.nextx = player.x;
	player.nexty = player.y;
	player.nextz = player.z;
	
	
	player.delta_x += player.speed_x;
	player.delta_y += player.speed_y;
	player.delta_z += player.speed_z;
	
	player.nextx += (player.delta_x*jo_cos(cam_angle_y) + player.delta_z*jo_sin(cam_angle_y))/32768;
	player.nexty += player.delta_y;
	player.nextz += (player.delta_z*jo_cos(cam_angle_y) - player.delta_x*jo_sin(cam_angle_y))/32768;
		
	for(Uint16 i = 0; i < total_sections; i++)
	{
	//set map section distance	
	x_dist = JO_ABS(player.x - map_section[i].x);
	y_dist = JO_ABS(player.y - map_section[i].y);
	z_dist = JO_ABS(player.z - map_section[i].z);
	
	section_dist = x_dist + y_dist + z_dist;
	
	//if(section_dist < COLL_DIST)
	//{
		collpoints_total = map_section[i].a_cdata->nbCo;
		
		for(Uint16 c = 0; c < collpoints_total; c++)
		{
			collpoints_x = map_section[i].a_collison[c].cen_x + map_section[i].x + map_section[i].tx;
			collpoints_y = map_section[i].a_collison[c].cen_y + map_section[i].y + map_section[i].ty;
			collpoints_z = map_section[i].a_collison[c].cen_z + map_section[i].z + map_section[i].tz;
			
			collpoints_xsize = map_section[i].a_collison[c].x_size;
			collpoints_ysize = map_section[i].a_collison[c].y_size;
			collpoints_zsize = map_section[i].a_collison[c].z_size;
			collpoints_type = map_section[i].a_collison[c].att;
				
		
			//VERTICAL COLLISION
			if(!ycollide)
			{
				//collide = has_vertical_collision(&map_section[i],player.int_height, player.x,player.y,player.z,player.xsize,player.ysize,player.zsize);
				collide = has_vertical_collision(collpoints_type, collpoints_x, collpoints_y, collpoints_z, collpoints_xsize, collpoints_ysize, collpoints_zsize,player.int_height, player.x,player.nexty,player.z,player.xsize,player.ysize,player.zsize);
				if(collide != NO_RAMP_COLLISION)
				{ 
					player.int_height = collide;
					
					if(player.speed_y <= 0.0f)
					{player.can_jump = true;
					}
					else
					{
					player.speed_y = 0.0f;
					delta_y = 0.0f;
					player.nexty = player.int_height;
					
							
					}
					
					player.current_map_section = i;
					player.current_collision = c;
				ycollide = true;//break;
				
				if(!show_debug && player.type == 4 && !player.in_water)
					{
						change_player_type(0);
					}
				}
				else 
				{
					if (i == player.current_map_section && c == player.current_collision)
					{
						if (!player.on_ladder_x && !player.on_ladder_z &&!player.on_ceiling )
						{
						player.can_jump = false;
						apply_player_gravity();
						
						player.rx = 0;	
						player.rz = 0;	
						}
					}
					
				}
				/*if(show_debug)
			{
			jo_nbg2_printf(0, 3, "map section %d ", i);
			jo_nbg2_printf(0, 5, "ycollide %4d" ,(int) ycollide);
			jo_nbg2_printf(0, 6, "can jump %4d" ,(int) player.can_jump);
			}*/
			
			
			}
		
								
				///check for shadow collision
				//collide = has_shadow_collision(&map_section[i], player.x,player.y + player.ysize,player.z,player.xsize,player.ysize,player.zsize);
				collide = has_shadow_collision(collpoints_x, collpoints_y, collpoints_z, collpoints_xsize, collpoints_ysize, collpoints_zsize, player.x,player.y + player.ysize,player.z,player.xsize,player.ysize,player.zsize);
				if(collide != NO_RAMP_COLLISION)
				{ 
				player.shadow_y = player.y + collide;
				player.shadow_size = 1-(JO_ABS(player.y - player.shadow_y)/100);
				player.current_shadow_map_section = i;
				player.current_shadow_collision = c;
				
				}
				else
				{	
					if (i == player.current_shadow_map_section && c == player.current_shadow_collision)
					{
					player.shadow_size = -1;	
					}
				}
			
					
			///check for X axis collision
			if(!xcollide)
			{
				//collide = has_horizontal_collision(&map_section[i],player.nextx,player.y,player.z,player.xsize,player.ysize,player.zsize);
				collide = has_horizontal_collision(collpoints_type, collpoints_x, collpoints_y, collpoints_z, collpoints_xsize, collpoints_ysize, collpoints_zsize,player.nextx,player.y,player.z,player.xsize,player.ysize,player.zsize);
				if(collide != NO_RAMP_COLLISION)
				{ 
					//spider climbs walls
					if(player.type == 1 && !player.on_ceiling)//&& !player.on_ceiling //add for section fix
					{
					player.on_ladder_x = true;
						if(player.speed_x >0)
						{
						//player.rx = -45;
						player.rz = -90;
						player.speed_y = -player.speed_x;
						}else if (player.speed_x <0)
						{
						player.rz = 90;
						player.speed_y = player.speed_x;
						}
					}
					
					player.nextx = player.x;
					
					//player.current_map_section = i;
					xcollide = true;
					
				}else
				{
					//if (i == player.current_map_section)
					//{
					player.on_ladder_x = false;
					//}
				}
			}
			
		
		
				///check for Z axis collision
			if(!zcollide)
			{
				//collide = has_horizontal_collision(&map_section[i],player.x,player.y,player.nextz,player.xsize,player.ysize,player.zsize);
				collide = has_horizontal_collision(collpoints_type, collpoints_x, collpoints_y, collpoints_z, collpoints_xsize, collpoints_ysize, collpoints_zsize,player.x,player.y,player.nextz,player.xsize,player.ysize,player.zsize);
				if(collide != NO_RAMP_COLLISION)
				{ 
					//spider climbs walls
					if(player.type == 1 )//&& !player.on_ceiling // add for section fix
					{
					player.on_ladder_z = true;
						if(player.speed_z >0)
						{
						player.rx = 90;
						player.speed_y = -player.speed_z;
						}else if (player.speed_z <0)
						{
						player.rx = -90;
						player.speed_y = player.speed_z;
						}
						
						
						
					}
				
				player.nextz = player.z;
				//player.current_map_section = i;
				zcollide = true;
					
				}else
				{
					//if (i == player.current_map_section)
					//{
					player.on_ladder_z = false;	
					//}
				}
			
			}
		
		
		
			if(!ccollide && collpoints_type == 0)
			{
				//CEILING COLLISION
				//collide = has_ceiling_collision(&map_section[i],player.x,player.y,player.z,player.xsize,player.ysize,player.zsize,player.speed_y);
				collide = has_ceiling_collision(collpoints_x, collpoints_y, collpoints_z, collpoints_xsize, collpoints_ysize, collpoints_zsize,player.x,player.y,player.z,player.xsize,player.ysize,player.zsize,player.speed_y);
				if(collide != NO_RAMP_COLLISION)
				{
				
					if(player.type ==1) //(!player.on_ladder_x && !player.on_ladder_z)
					{
					 
						if(player.speed_x !=0 )
						{
						player.on_ceiling = true;
						//player.on_ladder_x = false;
						player.rz = 180;
						player.nextx = player.x;//add for section fix
						player.nextx += ((-player.delta_x)*jo_cos(cam_angle_y) + player.delta_z*jo_sin(cam_angle_y))/32768;
						player.speed_y = 0.0f;
						}else if(player.speed_z !=0)
						{
						player.on_ceiling = true;
						player.rx = 180;
						player.nextz -= player.speed_z;
						player.speed_y = 0.0f;
						}else
						{
						player.speed_y = 0.0f;
						delta_y = 0.0f;
						apply_player_gravity();
						player.on_ceiling = false;	///
						}
						
					
					}else
					{
					player.speed_y = 0.0f;
					delta_y = 0.0f;
					apply_player_gravity();
					player.on_ceiling = false;///
					
					}
					
					
					player.current_map_section = i;
					ccollide = true;
				}else
				{	
					
					//player.on_ceiling = false;//remove for section fix
					
				}
			}
			
			
			
			///WATER
			if(!wcollide && collpoints_type == 9)
			{
			collide = has_object_collision(player.x, player.y, player.z, player.xsize, player.ysize, player.zsize, player.speed_x,player.speed_z,collpoints_x, collpoints_y, collpoints_z, collpoints_xsize, collpoints_ysize, collpoints_zsize);
			
				if(collide)
				{
					if(!player.in_water)
					{
						effect(0,player.x,player.y,player.z);
						player.in_water = true;
						player.speed_y = wmax_gravity;
						delta_y = wmax_gravity;
						apply_player_gravity();
						//}
					}
					if(!show_debug && (player.type == 1 || player.type == 2))
					{
					change_player_type(0);	
					}
					wcollide = true;
				}//else
				//{
				//player.in_water = false;
				//}
			
			}
		
		}	
		
		
	//}	
	}
	
	
	///has enemy collision
	for(Uint16 e = 0; e < enemy_total; e++)
	{
		
		if(enemies[e].health>0)
		{
			if(player.special_attack)
			{
			collide = has_object_collision(player.x, player.y, player.z, player.xsize*4, player.ysize, player.zsize, player.speed_x,player.speed_z,
								enemies[e].x, enemies[e].y, enemies[e].z, enemies[e].xsize, enemies[e].ysize, enemies[e].zsize);
			}else
			{
			collide = has_object_collision(player.x, player.y, player.z, player.xsize, player.ysize, player.zsize, player.speed_x,player.speed_z,
								enemies[e].x, enemies[e].y, enemies[e].z, enemies[e].xsize, enemies[e].ysize, enemies[e].zsize);	
			}
			//player stomp on enemy
			if(collide && player.speed_y >0 && player.type != 4)
			{
				
				enemies[e].health--;
				enemies[e].effect_size = 0; 
				
				player_bounce();
									
			}else //player collide with enemy
			if(collide && player.can_be_hurt)
			{
				if(player.special_attack)
			{
			stop_sounds();
			jo_audio_play_sound_on_channel(&die_sound, 3);
			enemies[e].health--;
			enemies[e].effect_size = 0;
			
			}else
				{
				player.can_be_hurt = false;	
				player.health--;
				player_bounceback();
				}
				
									
			}
		
		}
	}
	
	///has floor collision
	if(player.y >=132 && player.can_be_hurt)
	{
	player.can_be_hurt = false;	
	player.health --;	
	player_bounceback();
	}
		
	
	
	
	///has powerup collision
	for(Uint16 p = 0; p < powerup_total; p++)
	{
		
		if(!powerups[p].used)
		{	
			collide = has_object_collision(player.x, player.y, player.z, player.xsize, player.ysize, player.zsize, player.speed_x,player.speed_z,
								powerups[p].x, powerups[p].y, powerups[p].z, 16, 16, 16);
			
			if(collide)
			{		
							
					switch(powerups[p].type)
					{
							
					case 1:	
							if(player.health < 3)
							{
								stop_sounds();
								jo_audio_play_sound_on_channel(&pup_sound, 2);
								powerups[p].used = true;
								effect(1,powerups[p].x, powerups[p].y, powerups[p].z);
								player.health++;
							}
							break;
							
					case 2:	
							
								stop_sounds();
								jo_audio_play_sound_on_channel(&pup_sound, 2);
								powerups[p].used = true;
								effect(1,powerups[p].x, powerups[p].y, powerups[p].z);
								player.gems++;
							
							break;
							
					case 3:	
							
								stop_sounds();
								jo_audio_play_sound_on_channel(&pup_sound, 2);
								player.ry ++;
								player.nextx = powerups[p].x;
								player.nexty = powerups[p].y - (player.ysize*2);
								player.nextz = powerups[p].z;
								change_player_type(0);
								game.game_state = GAMESTATE_END_LEVEL;
								game.end_level_menu = 0;
														
							
							break;
							
					case 4:	
							
								stop_sounds();
								jo_audio_play_sound_on_channel(&pup_sound, 2);
								powerups[p].used = true;
								change_player_type(1);
								player.pup_timer = 0;
								player.mutate = true;
								effect(1,powerups[p].x, powerups[p].y, powerups[p].z);
								
							break;
							
					case 5:	
							
								stop_sounds();
								jo_audio_play_sound_on_channel(&pup_sound, 2);
								powerups[p].used = true;
								change_player_type(2);
								player.pup_timer = 0;
								player.mutate = true;
								effect(1,powerups[p].x, powerups[p].y, powerups[p].z);

							break;
				
					case 6:	
							
								stop_sounds();
								jo_audio_play_sound_on_channel(&pup_sound, 2);
								powerups[p].used = true;
								change_player_type(4);
								player.pup_timer = 0;
								player.mutate = true;
								effect(1,powerups[p].x, powerups[p].y, powerups[p].z);
								
							break;
					
					case 7:	
							
								stop_sounds();
								jo_audio_play_sound_on_channel(&pup_sound, 2);
								powerups[p].used = true;
								change_player_type(3);
								player.pup_timer = 0;
								player.mutate = true;
								effect(1,powerups[p].x, powerups[p].y, powerups[p].z);
								
							break;
				
					}
				
				
				
			}
			
		}
		
	}
	
	
	if(show_debug)
					{
					
					jo_nbg2_printf(0, 7, "NEXTX  %2d" ,player.nextx - player.x);
					
					
					}
					
	if(show_debug)
					{
					
					jo_nbg2_printf(0, 5, "LADDER  %d %d" ,(int) player.on_ladder_x, xcollide);
					jo_nbg2_printf(0, 6, "CEILING %d %d" ,(int) player.on_ceiling, ccollide);
					
					}
	
	
	//if(!ycollide)
	//{
	//apply_player_gravity();	
//	}
	
	
	
	player.x = player.nextx;
	player.y = player.nexty;
	player.z = player.nextz;
	
	
	
	
	//camera
	
	
	if(player.y <=0)
	{
	cam_pos_y = player.y + cam_height;//+ JO_DIV_BY_2(player.y)
	
	cam_target_y = player.y;
	}else
	{
		cam_pos_y = cam_height;
	}
	
	cam_pos_x = player.x + cam_adj_x;
	cam_pos_z = player.z + cam_adj_z;
	
	cam_target_x = player.x;
	cam_target_z = player.z;
	//cam_angle_y = player.ry;
	//player.on_ladder = false;
	
	if(!wcollide)
	{
		if(player.in_water)
		{
		effect(0,player.x,player.y,player.z);
		}
	player.in_water = false;	
	
	}
	
	if(!ccollide)
	{
	player.on_ceiling = false;	
	}
	
	if(player.mutate)
	{
	pup_timer_counter();	
	}
	
		
}



void create_enemy(enemy* new_enemy, Uint8 type, Sint16 x,Sint16 y, Sint16 z, Sint16 xdist, Sint16 zdist, Sint16 max_speed, Sint16 health)
{
	new_enemy->alive = true;
	new_enemy->type = type;
	new_enemy->start_health = health;
	new_enemy->health = health;
	new_enemy->can_be_hurt = true;
	new_enemy->start_x = x;
	new_enemy->start_y = y;
	new_enemy->start_z = z;
    new_enemy->x = x;
	new_enemy->y = y;
	new_enemy->z = z;
	new_enemy->xdist = xdist;
	new_enemy->zdist = zdist;
	new_enemy->max_speed = max_speed * framerate;
	new_enemy->explosion_size = 0;
	new_enemy->px = x;
	new_enemy->py = y;
	new_enemy->pz = z;
	new_enemy->shoot_wait = 0;
	new_enemy->anim_speed = ANIM_SPEED * framerate;
	

	if(type == 1)//spider
	{new_enemy->xsize = 19;
	new_enemy->ysize = 21;
	new_enemy->zsize= 19;
	}
	
	if(type == 2)//bat
	{new_enemy->xsize = 19;
	new_enemy->ysize = 21;
	new_enemy->zsize= 19;
	}
	if(type == 3)//frog
	{new_enemy->xsize = 19;
	new_enemy->ysize = 21;
	new_enemy->zsize= 19;
	//new_enemy->jump_height = -12.0f;
	new_enemy->rleg_rz = -120;
	new_enemy->lleg_rz = 128;
	new_enemy->jump_timer = ENEMY_JUMP_TIMER;
	}
	
}

void create_powerup(powerup* new_powerup, Uint8 type, Sint16 x,Sint16 y, Sint16 z)
{
	new_powerup->used = false;
	new_powerup->type = type;
	new_powerup->x = x;
	new_powerup->y = y;
	new_powerup->z = z;
	//  Set Model
	new_powerup->pup_model=(XPDATA *)pup_data[type-1];
	
	
}


/*void                create_waterfall(void)
{
	jo_start_sprite_anim_loop(wfall_anim);
    
}*/

void create_player(void)
{
	
   
	//  Set alive
    player.alive = true;
	player.health = 100;
	
	// set size (hit box)
	player.xsize = 16;//16
	player.ysize = 16;//32
	player.zsize = 16;//16
	player.speed = PLAYER_SPEED * framerate;
	player.jump_height = -6.0f;
	player.anim_speed = ANIM_SPEED * framerate;
		
	player.start_x = 0 ;
	player.start_y = 0;
	player.start_z = 0;
	
		
	//set gamepad
	player.gamepad = 0;
		
}

void create_map_section(level_section* section, Uint8 type, Sint16 x, Sint16 y, Sint16 z)
{
	// set type
	section->type = type;
	//  Set Location
    section->x = x;
    section->y = y;
    section->z = z;
	
			
	//  Set Model
	section->map_model=(XPDATA *)xpdata_[type];
	section->map_model_lp=(PDATA *)pdata_LP_[type];
	
	//	Set Collision Data
	section->a_cdata=(CDATA *)cdata_[type];
	section->a_collison	=(COLLISON *)section->a_cdata->cotbl;
		
				
}

/**Taken from RB demo**/
static FIXED light[XYZ];
static ANGLE light_ang[XYZ];
void computeLight()
{
    FIXED light_init[XYZ] = { toFIXED(0.57735026), toFIXED(0.57735026), toFIXED(0.57735026) };

    slPushUnitMatrix();
    {

        slRotX(light_ang[X]);
        slRotY(light_ang[Y]);
        slRotZ(light_ang[Z]);
        //slTranslate(toFIXED(15.0), toFIXED(-50.0), 0);
        slCalcVector(light_init, light);
    }
    slPopMatrix();
	///maybe useful for 1st person?
	//light_ang[X] = DEGtoANG(player.rx);
    //light_ang[Y] = DEGtoANG(player.ry);
    //light_ang[Z] = DEGtoANG(player.rz);
   
   if(player.in_water)
   {
   light_ang[X] += DEGtoANG(0.5);
   light_ang[Y] += DEGtoANG(0.5);
   light_ang[Z] += DEGtoANG(0.5);
   }else
   {	
	light_ang[X] = DEGtoANG(90.0);
    light_ang[Y] = DEGtoANG(90.0);
    light_ang[Z] = DEGtoANG(90.0);
   }
}

//FIXED tz=toFIXED(60.0);

void				animate_player(void)
{

//jump
	if(player.speed_y != 0)
	{
		
		if(player.type == 3)
		{
		if(player.rleg_rz < 0)
				player.rleg_rz+=32;
			
			if(player.lleg_rz > 0)
				player.lleg_rz-=32;
		}
	if(player.type == 4)
	{
		if(player.speed_y <0)
		{
		//	player.head_rx = -90;
			//player.larm_rx = -90;
		//	player.rarm_rx = -90;
		}else if(!player.in_water)
		{
			player.head_rx = 90;
			player.larm_rx = 90;
			player.rarm_rx = 90;
		}else
		{
			player.head_rx = 0;
			player.larm_rx = 0;
			player.rarm_rx = 0;
		}
	}else
	{
		player.head_rx = -15;
	}
		
	}
	else
	{
	player.head_rx = 0;	
	player.rarm_rz = 0;
	
	if(player.type == 3)
		{
		if(player.rleg_rz > -120)
				player.rleg_rz-=16;
			
		if(player.lleg_rz < 128)
			player.lleg_rz+=16;
		}
	}
	
	
	
	//jo_nbg2_printf(0, 0, "Ladder %d &d", player.on_ladder_x, player.on_ladder_z);
	
	if(player.special_attack)
	{
		switch(player.type)
		{
			case 0:	
					if(!player.can_jump)
					{
					//player.rx = 90;	
					}else
					{
					player.ry += 40; //hurricane spin attack
					player.larm_rx = 90;
					player.rarm_rx = -90;
					}
					break;
					
			case 1:	player.ry += 20;
					player.larm_rx = 90;
					player.rarm_rx = -90;
					break;
					
			case 2:	player.ry += 20;
					player.larm_rx = 90;
					player.rarm_rx = -90;
					break;
					
			case 3:	player.ry += 20;
					player.larm_rx = 90;
					player.rarm_rx = -90;
					break;
					
			case 4:	
					if(player.speed_x < 0) //dash attack
					{	
					player.speed_x = -8.0f;
					}
					else if(player.speed_x > 0)
					{
					player.speed_x = 8.0f;
					}
					else if(player.speed_z < 0)
					{
					player.speed_z = -8.0f;
					}
					else if(player.speed_z > 0)
					{
					player.speed_z = 8.0f;
					}
					else if(player.speed_y < 0)
					{
					player.speed_y = -8.0f;
					
					player.head_rx = -90;
					player.larm_rx = -90;
					player.rarm_rx = -90;
					}
					break;
		}
		
		
	}else if(player.speed_x != 0 && (player.speed_y == 0 || player.on_ladder_x || (player.in_water && player.type == 4)))//walk x 
		{
			
			if(player.head_ry == -18)
			{player.anim_speed = ANIM_SPEED * framerate;
			}
			else
			if(player.head_ry == 18)
			{player.anim_speed = -ANIM_SPEED * framerate;
			}
		
			player.head_ry += player.anim_speed;
			player.body_ry += player.anim_speed;
			player.larm_rx += player.anim_speed;
			player.rarm_rx -= player.anim_speed;
			player.lleg_rx -= player.anim_speed;
			player.rleg_rx += player.anim_speed;
			
			
			
				if(player.lleg_rx == 18 && player.type!=4)
				{
				//foot on floor - make walking sound and add dust clouds
				jo_audio_play_sound_on_channel(&step_sound, 2);
				player.left_cloud_x = player.x;
				player.left_cloud_y = player.y;
				player.left_cloud_z = player.z;
				player.left_cloud_size = 0.0f;
				
				}
				if(player.rleg_rx == 18 &&player.type!=4)
				{
				//foot on floor - make walking sound and add dust clouds
				jo_audio_play_sound_on_channel(&step_sound, 2);
				player.right_cloud_x = player.x;
				player.right_cloud_y = player.y;
				player.right_cloud_z = player.z;
				player.right_cloud_size = 0.0f;
				
				}
			
		}else if(player.speed_z != 0 && (player.speed_y == 0 || player.on_ladder_z || (player.in_water && player.type == 4)))//walk z 
		{
			if(player.head_ry == -18)
			{player.anim_speed = ANIM_SPEED * framerate;
			}
			else
			if(player.head_ry == 18)
			{player.anim_speed = -ANIM_SPEED * framerate;
			}
		
			player.head_ry += player.anim_speed;
			player.body_ry += player.anim_speed;
			
			player.larm_rx += player.anim_speed;
			player.rarm_rx -= player.anim_speed;
			player.lleg_rx -= player.anim_speed;
			player.rleg_rx += player.anim_speed;
			
			if(player.lleg_rx == 18 && player.type != 4)
			{
			//foot on floor - make walking sound and add dust clouds
			jo_audio_play_sound_on_channel(&step_sound, 2);
			player.left_cloud_x = player.x;
			player.left_cloud_y = player.y;
			player.left_cloud_z = player.z;
			player.left_cloud_size = 0.0f;
			
			}
			if(player.rleg_rx == 18 && player.type !=4)
			{
			//foot on floor - make walking sound and add dust clouds
			jo_audio_play_sound_on_channel(&step_sound, 2);
			player.right_cloud_x = player.x;
			player.right_cloud_y = player.y;
			player.right_cloud_z = player.z;
			player.right_cloud_size = 0.0f;
			
			}
			
			
		}
		else if(player.flapping)
		{
				
				/*if(current_enemy->head_rx <= -15)
				{current_enemy->anim_speed = 5;
				}
				else
				if(current_enemy->head_rx >= 15)
				{current_enemy->anim_speed = -5;
				}*/
				
				player.anim_speed = -5;
			
				player.head_rx += player.anim_speed;
				player.larm_rz -= player.anim_speed;
				player.rarm_rz += player.anim_speed;
				
				
				if(player.rarm_rz == -45 )
				{
				//flap - make flapping sound
				//stop_sounds();
				//jo_audio_play_sound_on_channel(&flap_sound, 4);
				player.flapping = false;
				player.larm_rz = 0;
				player.rarm_rz = 0;
				player.head_rx = 0;
				
				}/*else
				{
				jo_nbg2_printf(0, 0, "    ");	
				}*/
			}else
		{	player.head_ry = 0;
			player.body_ry = 0;
			player.body_rx = 0;
			player.lleg_rx = 0;
			player.rleg_rx = 0;
			if(player.type !=4)
			{
			player.larm_rx = 0;
			player.rarm_rx = 0;
			}
			
			
		}
		
		if(player.special_attack)
	{
		player.special_attack_timer += 1*framerate;
		
		if(player.special_attack_timer >= 18)
		{player.special_attack_timer = 0;
			player.special_attack = false;
		}
	}
	
	if(player.type ==4 && player.left_cloud_size >=1.0f)
			{
			player.left_cloud_x = player.x;
			player.left_cloud_y = player.y;
			player.left_cloud_z = player.z;
			player.left_cloud_size = 0.0f;
			
			}
		
	
	
	
	
	
	
}

void				animate_enemies(enemy* current_enemy)
{
	
if(current_enemy->health>0)
{
switch(current_enemy->type)

{
	
	
	case 1:
	//jump
		if(current_enemy->speed_y != 0)
		{
		current_enemy->body_rx = -15;	
		}
		else
		{
		current_enemy->body_rx = 0;	
		}
		

		if(current_enemy->speed_x != 0 && current_enemy->speed_y == 0)//walk x 
			{
				
				if(current_enemy->body_ry == -15)
				{current_enemy->anim_speed = ANIM_SPEED * framerate;
				}
				else
				if(current_enemy->body_ry == 15)
				{current_enemy->anim_speed = -ANIM_SPEED * framerate;
				}
			
				current_enemy->body_ry += current_enemy->anim_speed;
				current_enemy->larm_rx += current_enemy->anim_speed;
				current_enemy->rarm_rx -= current_enemy->anim_speed;
				current_enemy->lleg_rx -= current_enemy->anim_speed;
				current_enemy->rleg_rx += current_enemy->anim_speed;
				
				if(current_enemy->rleg_rx == 0 || current_enemy->lleg_rx == 0)
				{
				//foot on floor - make walking sound and add dust clouds
				//jo_nbg2_printf(0, 0, "STEP");
				jo_audio_play_sound_on_channel(&step_sound, 2);
				
				}/*else
				{
				jo_nbg2_printf(0, 0, "    ");	
				}*/
			}
		else if(current_enemy->speed_z != 0 && current_enemy->speed_y == 0)//walk z 
			{
				if(current_enemy->body_ry == -15)
				{current_enemy->anim_speed = ANIM_SPEED * framerate;
				}
				else
				if(current_enemy->body_ry == 15)
				{current_enemy->anim_speed = -ANIM_SPEED * framerate;
				}
			
				current_enemy->body_ry += current_enemy->anim_speed;
				
				current_enemy->larm_rx += current_enemy->anim_speed;
				current_enemy->rarm_rx -= current_enemy->anim_speed;
				current_enemy->lleg_rx -= current_enemy->anim_speed;
				current_enemy->rleg_rx += current_enemy->anim_speed;
				
				if(current_enemy->rleg_rx == 0 || current_enemy->lleg_rx == 0)
				{
				//foot on floor - make walking sound and add dust clouds
				//jo_nbg2_printf(0, 0, "STEP");
				jo_audio_play_sound_on_channel(&step_sound, 2);
				
				}/*else
				{
				jo_nbg2_printf(0, 0, "    ");	
				}*/
			}
			else
			{	current_enemy->body_ry = 0;
				current_enemy->body_rx = 0;
				current_enemy->larm_rx = 0;
				current_enemy->rarm_rx = 0;
				current_enemy->lleg_rx = 0;
				current_enemy->rleg_rx = 0;
				
			}
		break;
		
	case 2:
	
		//jump
		/*if(current_enemy->speed_y != 0)
		{
		current_enemy->head_rx = -15;	
		}
		else
		{
		current_enemy->head_rx = 0;	
		}*/
		

			if(current_enemy->speed_x != 0 || current_enemy->speed_z != 0)//fly
			{
				
				if(current_enemy->head_rx <= -45)
				{current_enemy->anim_speed = 5;
				}
				else
				if(current_enemy->head_rx >= 45)
				{current_enemy->anim_speed = -5;
				}
			
				current_enemy->head_rx += current_enemy->anim_speed;
				current_enemy->larm_rz -= current_enemy->anim_speed;
				current_enemy->rarm_rz += current_enemy->anim_speed;
				
				
				if(current_enemy->rarm_rz == 45 )
				{
				//flap - make flapping sound
				//stop_sounds();
				jo_audio_play_sound_on_channel(&flap_sound, 4);
				
				}/*else
				{
				jo_nbg2_printf(0, 0, "    ");	
				}*/
			}
			else
			{	current_enemy->head_rx = 0;
				current_enemy->larm_rz = 0;
				current_enemy->rarm_rz = 0;
			
				
			}
			break;
			
			case 3:
	//jump
		if(current_enemy->speed_y != 0)
		{
		current_enemy->body_rx = -15;	
			if(current_enemy->rleg_rz < 0)
				current_enemy->rleg_rz+=32;
			
			if(current_enemy->lleg_rz > 0)
				current_enemy->lleg_rz-=32;
		}
		else
		{
		current_enemy->body_rx = 0;	
		
		if(current_enemy->rleg_rz > -120)
				current_enemy->rleg_rz-=16;
			
		if(current_enemy->lleg_rz < 128)
			current_enemy->lleg_rz+=16;
		
		}
		
		
		if(current_enemy->y >= current_enemy->start_y)
				{ 
										
					if(current_enemy->speed_y <= 0.0f)
					{current_enemy->can_jump = true;
					}
					else
					{
					current_enemy->speed_y = 0.0f;
										}
					
				}
				else 
				{	
					current_enemy->can_jump = false;
					if (current_enemy->speed_y < max_gravity)
						{
						current_enemy->speed_y += gravity;
						}
						
					
				}
				
		if(current_enemy->can_jump)
		{
			current_enemy->jump_timer -= 1*framerate;	
		}else
		{
		current_enemy->jump_timer = ENEMY_JUMP_TIMER;
		
		}
		
		if(current_enemy->jump_timer <= 0)
		{
		stop_sounds();
		jo_audio_play_sound_on_channel(&boing_sound, 0);
		current_enemy->speed_y = -8.0f;
		}
			
		
		
		

		if(current_enemy->speed_x != 0 && current_enemy->speed_y == 0)//walk x 
			{
				
				if(current_enemy->body_ry == -15)
				{current_enemy->anim_speed = ANIM_SPEED * framerate;
				}
				else
				if(current_enemy->body_ry == 15)
				{current_enemy->anim_speed = -ANIM_SPEED * framerate;
				}
			
				current_enemy->body_ry += current_enemy->anim_speed;
				current_enemy->larm_rx += current_enemy->anim_speed;
				current_enemy->rarm_rx -= current_enemy->anim_speed;
				current_enemy->lleg_rx -= current_enemy->anim_speed;
				current_enemy->rleg_rx += current_enemy->anim_speed;
				
				if(current_enemy->rleg_rx == 0 || current_enemy->lleg_rx == 0)
				{
				//foot on floor - make walking sound and add dust clouds
				//jo_nbg2_printf(0, 0, "STEP");
				jo_audio_play_sound_on_channel(&step_sound, 2);
				
				}/*else
				{
				jo_nbg2_printf(0, 0, "    ");	
				}*/
			}
		else if(current_enemy->speed_z != 0 && current_enemy->speed_y == 0)//walk z 
			{
				if(current_enemy->body_ry == -15)
				{current_enemy->anim_speed = ANIM_SPEED * framerate;
				}
				else
				if(current_enemy->body_ry == 15)
				{current_enemy->anim_speed = -ANIM_SPEED * framerate;
				}
			
				current_enemy->body_ry += current_enemy->anim_speed;
				
				current_enemy->larm_rx += current_enemy->anim_speed;
				current_enemy->rarm_rx -= current_enemy->anim_speed;
				current_enemy->lleg_rx -= current_enemy->anim_speed;
				current_enemy->rleg_rx += current_enemy->anim_speed;
				
				if(current_enemy->rleg_rx == 0 || current_enemy->lleg_rx == 0)
				{
				//foot on floor - make walking sound and add dust clouds
				//jo_nbg2_printf(0, 0, "STEP");
				jo_audio_play_sound_on_channel(&step_sound, 2);
				
				}/*else
				{
				jo_nbg2_printf(0, 0, "    ");	
				}*/
			}
			else
			{	current_enemy->body_ry = 0;
				current_enemy->body_rx = 0;
				current_enemy->larm_rx = 0;
				current_enemy->rarm_rx = 0;
				current_enemy->lleg_rx = 0;
				current_enemy->rleg_rx = 0;
				
			}
		break;
		
	case 4:
	//jump
		if(current_enemy->speed_y != 0)
		{
		current_enemy->body_rx = -15;	
			if(current_enemy->rleg_rz < 0)
				current_enemy->rleg_rz+=32;
			
			if(current_enemy->lleg_rz > 0)
				current_enemy->lleg_rz-=32;
		}
		else
		{
		current_enemy->body_rx = 0;	
		
		if(current_enemy->rleg_rz > -120)
				current_enemy->rleg_rz-=16;
			
		if(current_enemy->lleg_rz < 128)
			current_enemy->lleg_rz+=16;
		
		}
		
		
		if(current_enemy->y >= current_enemy->start_y)
				{ 
										
					if(current_enemy->speed_y <= 0.0f && !player.in_water)
					{current_enemy->can_jump = true;
					}
					else
					{
					current_enemy->speed_y = 0.0f;
										}
					
				}
				else 
				{	
					current_enemy->can_jump = false;
					if (current_enemy->speed_y < max_gravity)
						{
						current_enemy->speed_y += gravity;
						}
						
					
				}
				
		if(current_enemy->can_jump)
		{
			current_enemy->jump_timer -= 1*framerate;	
		}else
		{
		current_enemy->jump_timer = ENEMY_JUMP_TIMER;
		
		}
		
		if(current_enemy->jump_timer <= 0)
		{
		stop_sounds();
		jo_audio_play_sound_on_channel(&flap_sound, 0);
		current_enemy->speed_y = -8.0f * framerate;
		}
			
		
		
		

		if(current_enemy->speed_x != 0 && current_enemy->speed_y == 0)//walk x 
			{
				
				if(current_enemy->body_ry == -15)
				{current_enemy->anim_speed = ANIM_SPEED * framerate;
				}
				else
				if(current_enemy->body_ry == 15)
				{current_enemy->anim_speed = -ANIM_SPEED * framerate;
				}
			
				current_enemy->body_ry += current_enemy->anim_speed;
				current_enemy->larm_rx += current_enemy->anim_speed;
				current_enemy->rarm_rx -= current_enemy->anim_speed;
				current_enemy->lleg_rx -= current_enemy->anim_speed;
				current_enemy->rleg_rx += current_enemy->anim_speed;
				
				if(current_enemy->rleg_rx == 0 || current_enemy->lleg_rx == 0)
				{
				//foot on floor - make walking sound and add dust clouds
				//jo_nbg2_printf(0, 0, "STEP");
				//jo_audio_play_sound_on_channel(&step_sound, 2);
				
				}/*else
				{
				jo_nbg2_printf(0, 0, "    ");	
				}*/
			}
		else if(current_enemy->speed_z != 0 && current_enemy->speed_y == 0)//walk z 
			{
				if(current_enemy->body_ry == -15)
				{current_enemy->anim_speed = ANIM_SPEED * framerate;
				}
				else
				if(current_enemy->body_ry == 15)
				{current_enemy->anim_speed = -ANIM_SPEED * framerate;
				}
			
				current_enemy->body_ry += current_enemy->anim_speed;
				
				current_enemy->larm_rx += current_enemy->anim_speed;
				current_enemy->rarm_rx -= current_enemy->anim_speed;
				current_enemy->lleg_rx -= current_enemy->anim_speed;
				current_enemy->rleg_rx += current_enemy->anim_speed;
				
				if(current_enemy->rleg_rx == 0 || current_enemy->lleg_rx == 0)
				{
				//foot on floor - make walking sound and add dust clouds
				//jo_nbg2_printf(0, 0, "STEP");
				//jo_audio_play_sound_on_channel(&step_sound, 2);
				
				}/*else
				{
				jo_nbg2_printf(0, 0, "    ");	
				}*/
			}
			else
			{	current_enemy->body_ry = 0;
				current_enemy->body_rx = 0;
				current_enemy->larm_rx = 0;
				current_enemy->rarm_rx = 0;
				current_enemy->lleg_rx = 0;
				current_enemy->rleg_rx = 0;
				
			}
		break;
	
}

//move enemy around path
		
	if(current_enemy->x + current_enemy->z == current_enemy->start_x + current_enemy->start_z)
	{current_enemy->waypoint = 0;
	}
	
	if(current_enemy->x == current_enemy->start_x + current_enemy->xdist && current_enemy->z == current_enemy->start_z)
	{current_enemy->waypoint = 1;
	}
	
	if(current_enemy->x == current_enemy->start_x + current_enemy->xdist && current_enemy->z == current_enemy->start_z + current_enemy->zdist)
	{current_enemy->waypoint = 2;
	}
	
	if(current_enemy->x == current_enemy->start_x && current_enemy->z == current_enemy->start_z + current_enemy->zdist)
	{current_enemy->waypoint = 3;
	}
	
	if(current_enemy->waypoint == 0)
	{current_enemy->speed_x = 1;
	current_enemy->speed_z = 0;
	current_enemy->ry = -90;
	}else if(current_enemy->waypoint == 1)
	{current_enemy->speed_z = 1;
	current_enemy->speed_x = 0;
	current_enemy->ry = 180;
	}else if(current_enemy->waypoint == 2)
	{current_enemy->speed_x = -1;
	current_enemy->speed_z = 0;
	current_enemy->ry = 90;
	}else if(current_enemy->waypoint == 3)
	{current_enemy->speed_z = -1;
	current_enemy->speed_x = 0;
	current_enemy->ry = 0;
	}
	
	current_enemy->x += current_enemy->speed_x;
	current_enemy->y += current_enemy->speed_y;
	current_enemy->z += current_enemy->speed_z;
	
	
}
	if(current_enemy->health<= 0)
	{
		
	current_enemy->death_timer += 1*framerate;
	current_enemy->y+= 4;
	current_enemy->rz=180;
		if(current_enemy->death_timer>=20)
		{
		current_enemy->alive = false;	
		}
	
	}
}

void				draw_enemies(enemy* current_enemy)
{
	

	if(current_enemy->alive)
	{
		
	
	if(player.in_water)
	{
	slSetGouraudColor(JO_COLOR_RGB(71,245,249));
	}else if(current_enemy->health<= 0)
	{
	slSetGouraudColor(CD_Red);
	}else
	{
	slSetGouraudColor(CD_White);
	}
	
	
	
		
		switch(current_enemy->type)
		{
			case 1:
		
			slPushMatrix();
			{
				slTranslate(toFIXED(current_enemy->x), toFIXED(current_enemy->y), toFIXED(current_enemy->z));
				slRotX(DEGtoANG(current_enemy->rx)); slRotY(DEGtoANG(current_enemy->ry)); slRotZ(DEGtoANG(current_enemy->rz));
				slRotX(DEGtoANG(current_enemy->body_rx)); slRotY(DEGtoANG(current_enemy->body_ry)); slRotZ(DEGtoANG(current_enemy->body_rz));
				jo_3d_set_scalef(0.6,0.6,0.6);	
				
				slPutPolygonX((XPDATA *)&xpdata_spider_body, light);
			}
			slPopMatrix();
			
			slPushMatrix();
			{
				slTranslate(toFIXED(current_enemy->x), toFIXED(current_enemy->y), toFIXED(current_enemy->z));
				slRotX(DEGtoANG(current_enemy->rx)); slRotY(DEGtoANG(current_enemy->ry)); slRotZ(DEGtoANG(current_enemy->rz));
				slRotX(DEGtoANG(current_enemy->larm_rx)); slRotY(DEGtoANG(current_enemy->larm_ry)); slRotZ(DEGtoANG(current_enemy->larm_rz));
				jo_3d_set_scalef(0.6,0.6,0.6);	
				
				slPutPolygonX((XPDATA *)&xpdata_spider_LL1, light);
			}
			slPopMatrix();
			
			slPushMatrix();
			{
				slTranslate(toFIXED(current_enemy->x), toFIXED(current_enemy->y), toFIXED(current_enemy->z));
				slRotX(DEGtoANG(current_enemy->rx)); slRotY(DEGtoANG(current_enemy->ry)); slRotZ(DEGtoANG(current_enemy->rz));
				slRotX(DEGtoANG(current_enemy->rarm_rx)); slRotY(DEGtoANG(current_enemy->rarm_ry)); slRotZ(DEGtoANG(current_enemy->rarm_rz));
				jo_3d_set_scalef(0.6,0.6,0.6);	
				
				slPutPolygonX((XPDATA *)&xpdata_spider_RL1, light);
			}
			slPopMatrix();
			
			slPushMatrix();
			{
				slTranslate(toFIXED(current_enemy->x), toFIXED(current_enemy->y), toFIXED(current_enemy->z));
				slRotX(DEGtoANG(current_enemy->rx)); slRotY(DEGtoANG(current_enemy->ry)); slRotZ(DEGtoANG(current_enemy->rz));
				slRotX(DEGtoANG(current_enemy->lleg_rx)); slRotY(DEGtoANG(current_enemy->lleg_ry)); slRotZ(DEGtoANG(current_enemy->lleg_rz));
				jo_3d_set_scalef(0.6,0.6,0.6);	
				
				slPutPolygonX((XPDATA *)&xpdata_spider_LL2, light);
			}
			slPopMatrix();
			
			slPushMatrix();
			{
				slTranslate(toFIXED(current_enemy->x), toFIXED(current_enemy->y), toFIXED(current_enemy->z));
				slRotX(DEGtoANG(current_enemy->rx)); slRotY(DEGtoANG(current_enemy->ry)); slRotZ(DEGtoANG(current_enemy->rz));
				slRotX(DEGtoANG(current_enemy->rleg_rx)); slRotY(DEGtoANG(current_enemy->rleg_ry)); slRotZ(DEGtoANG(current_enemy->rleg_rz));
				jo_3d_set_scalef(0.6,0.6,0.6);	
				
				slPutPolygonX((XPDATA *)&xpdata_spider_RL2, light);
			}
			slPopMatrix();
			
			break;
			
			case 2:
		
			slPushMatrix();
			{
				slTranslate(toFIXED(current_enemy->x), toFIXED(current_enemy->y), toFIXED(current_enemy->z));
				slRotX(DEGtoANG(current_enemy->rx)); slRotY(DEGtoANG(current_enemy->ry)); slRotZ(DEGtoANG(current_enemy->rz));
				slRotX(DEGtoANG(current_enemy->head_rx)); slRotY(DEGtoANG(current_enemy->head_ry)); slRotZ(DEGtoANG(current_enemy->head_rz));
				
				slPutPolygonX((XPDATA *)&xpdata_bat_head, light);
			}
			slPopMatrix();
			
			slPushMatrix();
			{
				slTranslate(toFIXED(current_enemy->x), toFIXED(current_enemy->y), toFIXED(current_enemy->z));
				slRotX(DEGtoANG(current_enemy->rx)); slRotY(DEGtoANG(current_enemy->ry)); slRotZ(DEGtoANG(current_enemy->rz));
				slRotX(DEGtoANG(current_enemy->body_rx)); slRotY(DEGtoANG(current_enemy->body_ry)); slRotZ(DEGtoANG(current_enemy->body_rz));
				
				slPutPolygonX((XPDATA *)&xpdata_bat_body, light);
			}
			slPopMatrix();
			
			slPushMatrix();
			{
				slTranslate(toFIXED(current_enemy->x), toFIXED(current_enemy->y), toFIXED(current_enemy->z));
				slRotX(DEGtoANG(current_enemy->rx)); slRotY(DEGtoANG(current_enemy->ry)); slRotZ(DEGtoANG(current_enemy->rz));
				slRotX(DEGtoANG(current_enemy->larm_rx)); slRotY(DEGtoANG(current_enemy->larm_ry)); slRotZ(DEGtoANG(current_enemy->larm_rz));
				
				slPutPolygonX((XPDATA *)&xpdata_bat_lwing, light);
			}
			slPopMatrix();
			
			slPushMatrix();
			{
				slTranslate(toFIXED(current_enemy->x), toFIXED(current_enemy->y), toFIXED(current_enemy->z));
				slRotX(DEGtoANG(current_enemy->rx)); slRotY(DEGtoANG(current_enemy->ry)); slRotZ(DEGtoANG(current_enemy->rz));
				slRotX(DEGtoANG(current_enemy->rarm_rx)); slRotY(DEGtoANG(current_enemy->rarm_ry)); slRotZ(DEGtoANG(current_enemy->rarm_rz));
				
				slPutPolygonX((XPDATA *)&xpdata_bat_rwing, light);
			}
			slPopMatrix();
			
			
			
			break;
			
			
			case 3:	
	
						
				slPushMatrix();
				{
					slTranslate(toFIXED(current_enemy->x), toFIXED(current_enemy->y), toFIXED(current_enemy->z));
					slRotX(DEGtoANG(current_enemy->rx)); slRotY(DEGtoANG(current_enemy->ry)); slRotZ(current_enemy->rz);
					slPutPolygonX((XPDATA *)&xpdata_frog_body,light);
				
					//lleg
					slPushMatrix();
					{
						slTranslate(toFIXED(8.0), toFIXED(0), toFIXED(0));
						slRotX(DEGtoANG(current_enemy->rleg_rx)); slRotY(DEGtoANG(current_enemy->rleg_ry)); slRotZ(DEGtoANG(current_enemy->rleg_rz));
						slPutPolygonX((XPDATA *)&xpdata_frog_LLeg1,light);
					
						slPushMatrix();
						{
							slTranslate(toFIXED(0.0), toFIXED(16), toFIXED(0));
							slRotX(DEGtoANG(0.0)); slRotY(DEGtoANG(0.0)); slRotZ(DEGtoANG(current_enemy->lleg_rz));
							slPutPolygonX((XPDATA *)&xpdata_frog_LLeg2,light);
						slPushMatrix();
							{
								slTranslate(toFIXED(0.0), toFIXED(16), toFIXED(0));
							slRotX(DEGtoANG(0.0)); slRotY(DEGtoANG(0.0)); slRotZ(DEGtoANG(0.0));
								slPutPolygonX((XPDATA *)&xpdata_frog_Leg3,light);
							}
							
							slPopMatrix();
							
						}	
						slPopMatrix();
						
					}
					slPopMatrix();
					//rleg
					slPushMatrix();
					{
						slTranslate(toFIXED(-8.0), toFIXED(0), toFIXED(0));
						slRotX(DEGtoANG(current_enemy->lleg_rx)); slRotY(DEGtoANG(current_enemy->lleg_ry)); slRotZ(DEGtoANG(-current_enemy->rleg_rz));
						slPutPolygonX((XPDATA *)&xpdata_frog_RLeg1,light);
					
						slPushMatrix();
						{
							slTranslate(toFIXED(0.0), toFIXED(16), toFIXED(0));
							slRotX(DEGtoANG(0.0)); slRotY(DEGtoANG(0.0)); slRotZ(DEGtoANG(-current_enemy->lleg_rz));
							slPutPolygonX((XPDATA *)&xpdata_frog_RLeg2,light);
						slPushMatrix();
							{
								slTranslate(toFIXED(0.0), toFIXED(16), toFIXED(0));
							slRotX(DEGtoANG(0.0)); slRotY(DEGtoANG(0.0)); slRotZ(DEGtoANG(0.0));
								slPutPolygonX((XPDATA *)&xpdata_frog_Leg3,light);
							}
							
							slPopMatrix();
							
						}	
						slPopMatrix();
						
					}
					slPopMatrix();
					
					}
				slPopMatrix();
				
				break;
				
		case 4:
		
						
			slPushMatrix();
			{
				slTranslate(toFIXED(current_enemy->x), toFIXED(current_enemy->y), toFIXED(current_enemy->z));
				slRotX(DEGtoANG(current_enemy->rx)); slRotY(DEGtoANG(current_enemy->ry)); slRotZ(DEGtoANG(current_enemy->rz));
				slRotX(DEGtoANG(current_enemy->body_rx)); slRotY(DEGtoANG(current_enemy->body_ry)); slRotZ(DEGtoANG(current_enemy->body_rz));
				
				slPutPolygonX((XPDATA *)&xpdata_fish_body, light);
			}
			slPopMatrix();
			
			slPushMatrix();
			{
				slTranslate(toFIXED(current_enemy->x), toFIXED(current_enemy->y), toFIXED(current_enemy->z));
				slRotX(DEGtoANG(current_enemy->rx)); slRotY(DEGtoANG(current_enemy->ry)); slRotZ(DEGtoANG(current_enemy->rz));
				slRotX(DEGtoANG(current_enemy->larm_rx)); slRotY(DEGtoANG(current_enemy->larm_ry)); slRotZ(DEGtoANG(current_enemy->larm_rz));
				
				slPutPolygonX((XPDATA *)&xpdata_fish_dfin, light);
			}
			slPopMatrix();
			
			slPushMatrix();
			{
				slTranslate(toFIXED(current_enemy->x), toFIXED(current_enemy->y), toFIXED(current_enemy->z));
				slRotX(DEGtoANG(current_enemy->rx)); slRotY(DEGtoANG(current_enemy->ry)); slRotZ(DEGtoANG(current_enemy->rz));
				slRotX(DEGtoANG(current_enemy->rarm_rx)); slRotY(DEGtoANG(current_enemy->rarm_ry)); slRotZ(DEGtoANG(current_enemy->rarm_rz));
				
				slPutPolygonX((XPDATA *)&xpdata_fish_tfin, light);
			}
			slPopMatrix();
			
			//air bubbles
				if(current_enemy->air_bubble_size < 1.0f)
				{
					jo_sprite_enable_half_transparency();
					slPushMatrix();
					{
					slTranslate(toFIXED(current_enemy->air_bubble_x), toFIXED(current_enemy->air_bubble_y-8), toFIXED(current_enemy->air_bubble_z));
					jo_3d_set_scalef(current_enemy->air_bubble_size, current_enemy->air_bubble_size, current_enemy->air_bubble_size);
						//if(current_enemy->air_bubble_y >= player.effect_y)
						//{
						jo_3d_draw_billboard(8,0,0,0);//could replace with polygon as scale not working
						//}
					}
					slPopMatrix();
					jo_sprite_disable_half_transparency();
					
				
				current_enemy->air_bubble_size += 0.005f;
				current_enemy->air_bubble_y -= 0.25f;
				}
			
			break;
		}
	}
	
	if(current_enemy->effect_size <= 2.0f)
			{
				
						slPushMatrix();
						{
							
							slTranslate(toFIXED(current_enemy->x), toFIXED(current_enemy->y), toFIXED(current_enemy->z));
							jo_3d_set_scalef(current_enemy->effect_size,current_enemy->effect_size,current_enemy->effect_size);			
							slPutPolygonX((XPDATA *)&xpdata_die_effect,light);
						}
						slPopMatrix();
						
			
			current_enemy->effect_size +=0.2f;	
			
			}
			
	
	
	
	
	
}

void				draw_powerups(powerup* current_powerup)
{
	
	if((current_powerup->type == 4 || current_powerup->type == 5 || current_powerup->type == 6 || current_powerup->type == 7) && current_powerup->used == true && player.mutate == false)
				{
					current_powerup->used = false;
				}

	if(player.in_water)
	{
	slSetGouraudColor(JO_COLOR_RGB(71,245,249));
	}else
	{
	slSetGouraudColor(CD_White);
	}
	
	if(!current_powerup->used)
	{
		slPushMatrix();
		{
			slTranslate(toFIXED(current_powerup->x), toFIXED(current_powerup->y), toFIXED(current_powerup->z));
			slRotX(DEGtoANG(0)); slRotY(DEGtoANG(current_powerup->ry)); slRotZ(DEGtoANG(0));
			{
			slPutPolygonX(current_powerup->pup_model, light);
			}
		}
		slPopMatrix();
		
		
	}
	
	if(current_powerup->ry >=360)
	{
	current_powerup->ry = 0;
	}else
	{
	current_powerup->ry++;	
	}
}

void				draw_player(void)
{
	
	slSetGouraudColor(JO_COLOR_RGB(player.r, player.g, player.b));
	
	
	if(!player.can_be_hurt)
	{
	player.r = 255; player.g = 0; player.b = 0;
	}else if(player.effect_size <= 2.0f)
	{
	player.r = 228; player.g = 210; player.b = 242;	
	}else if(player.in_water)
	{
	player.r = 71; player.g = 245; player.b = 249;
	}else
	{
		player.r = 255; player.g = 255; player.b = 255;
	}
	
	switch(player.type)
		{
        
				
		case 0:	
	
				slPushMatrix();
				{
					slTranslate(toFIXED(player.x), toFIXED(player.y+4), toFIXED(player.z));
					slRotX(DEGtoANG(player.rx)); slRotY(DEGtoANG(player.ry)); slRotZ(DEGtoANG(player.rz));
					slRotX(DEGtoANG(player.head_rx)); slRotY(DEGtoANG(player.head_ry)); slRotZ(DEGtoANG(player.head_rz));
					
					slPutPolygonX((XPDATA *)&xpdata_ham_head, light);
				}
				slPopMatrix();
				
				slPushMatrix();
				{
					slTranslate(toFIXED(player.x), toFIXED(player.y+4), toFIXED(player.z));
					slRotX(DEGtoANG(player.rx)); slRotY(DEGtoANG(player.ry)); slRotZ(DEGtoANG(player.rz));
					slRotX(DEGtoANG(player.body_rx)); slRotY(DEGtoANG(player.body_ry)); slRotZ(DEGtoANG(player.body_rz));
					
					slPutPolygonX((XPDATA *)&xpdata_ham_body, light);
				}
				slPopMatrix();
				
				slPushMatrix();
				{
					slTranslate(toFIXED(player.x), toFIXED(player.y+4), toFIXED(player.z));
					slRotX(DEGtoANG(player.rx)); slRotY(DEGtoANG(player.ry)); slRotZ(DEGtoANG(player.rz));
					slRotX(DEGtoANG(player.larm_rx)); slRotY(DEGtoANG(player.larm_ry)); slRotZ(DEGtoANG(player.larm_rz));
					
					slPutPolygonX((XPDATA *)&xpdata_ham_larm, light);
				}
				slPopMatrix();
				
				slPushMatrix();
				{
					slTranslate(toFIXED(player.x), toFIXED(player.y+4), toFIXED(player.z));
					slRotX(DEGtoANG(player.rx)); slRotY(DEGtoANG(player.ry)); slRotZ(DEGtoANG(player.rz));
					slRotX(DEGtoANG(player.rarm_rx)); slRotY(DEGtoANG(player.rarm_ry)); slRotZ(DEGtoANG(player.rarm_rz));
					
					slPutPolygonX((XPDATA *)&xpdata_ham_rarm, light);
				}
				slPopMatrix();
				
				slPushMatrix();
				{
					slTranslate(toFIXED(player.x), toFIXED(player.y+4), toFIXED(player.z));
					slRotX(DEGtoANG(player.rx)); slRotY(DEGtoANG(player.ry)); slRotZ(DEGtoANG(player.rz));
					slRotX(DEGtoANG(player.lleg_rx)); slRotY(DEGtoANG(player.lleg_ry)); slRotZ(DEGtoANG(player.lleg_rz));
					
					slPutPolygonX((XPDATA *)&xpdata_ham_lleg, light);
				}
				slPopMatrix();
				
				slPushMatrix();
				{
					slTranslate(toFIXED(player.x), toFIXED(player.y+4), toFIXED(player.z));
					slRotX(DEGtoANG(player.rx)); slRotY(DEGtoANG(player.ry)); slRotZ(DEGtoANG(player.rz));
					slRotX(DEGtoANG(player.rleg_rx)); slRotY(DEGtoANG(player.rleg_ry)); slRotZ(DEGtoANG(player.rleg_rz));
					
					slPutPolygonX((XPDATA *)&xpdata_ham_rleg, light);
				}
				slPopMatrix();
				
				if(player.shadow_size >0)
				{
				slPushMatrix();
				{
					
					slTranslate(toFIXED(player.x), toFIXED(player.shadow_y), toFIXED(player.z));
					//slRotX(DEGtoANG(player.rx)); slRotY(DEGtoANG(player.ry)); slRotZ(DEGtoANG(player.rz));
					jo_3d_set_scalef(player.shadow_size,player.shadow_size,player.shadow_size);				
					slPutPolygon((PDATA *)&xpdata_ham_shadow);
				}
				slPopMatrix();
				}
				if(player.left_cloud_size < 1.0f)
				{
					jo_sprite_enable_half_transparency();
					slPushMatrix();
					{
					slTranslate(toFIXED(player.left_cloud_x - 8), toFIXED(player.left_cloud_y + 30), toFIXED(player.left_cloud_z));
					jo_3d_set_scalef(player.left_cloud_size, player.left_cloud_size, player.left_cloud_size);
					jo_3d_draw_billboard(5,0,0,0);
					//render_CLUT_sprite(5,0,0,0);
					//jo_3d_restore_scale();
					}
					slPopMatrix();
					jo_sprite_disable_half_transparency();
					
				
				player.left_cloud_size += 0.05f;
				player.left_cloud_y -= 1;
				}
				
				if(player.right_cloud_size < 1.0f)
				{
					jo_sprite_enable_half_transparency();
					slPushMatrix();
					{
					slTranslate(toFIXED(player.right_cloud_x + 8), toFIXED(player.right_cloud_y + 30), toFIXED(player.right_cloud_z));
					jo_3d_set_scalef(player.right_cloud_size, player.right_cloud_size, player.right_cloud_size);
					jo_3d_draw_billboard(5,0,0,0);
					//render_CLUT_sprite(5,0,0,0);
					//jo_3d_restore_scale();
					}
					slPopMatrix();
					jo_sprite_disable_half_transparency();
					
				
				player.right_cloud_size += 0.05f;
				player.right_cloud_y -= 1;
				}
				
				break;
				
		case 1:	
	
				slPushMatrix();
				{
					slTranslate(toFIXED(player.x), toFIXED(player.y), toFIXED(player.z));
					slRotX(DEGtoANG(player.rx)); slRotZ(DEGtoANG(player.rz));slRotY(DEGtoANG(player.ry));
					slRotX(DEGtoANG(player.head_rx)); slRotY(DEGtoANG(player.head_ry)); slRotZ(DEGtoANG(player.head_rz));
					jo_3d_set_scalef(0.6,0.6,0.6);	
					
					slPutPolygonX((XPDATA *)&xpdata_ham_head, light);
				}
				slPopMatrix();
				
								
				slPushMatrix();
				{
					slTranslate(toFIXED(player.x), toFIXED(player.y), toFIXED(player.z));
					slRotX(DEGtoANG(player.rx));  slRotZ(DEGtoANG(player.rz)); slRotY(DEGtoANG(player.ry));
					slRotX(DEGtoANG(player.larm_rx)); slRotY(DEGtoANG(player.larm_ry)); slRotZ(DEGtoANG(player.larm_rz));
					jo_3d_set_scalef(0.6,0.6,0.6);	
					
					slPutPolygonX((XPDATA *)&xpdata_ham_LL1, light);
				}
				slPopMatrix();
				
				slPushMatrix();
				{
					slTranslate(toFIXED(player.x), toFIXED(player.y), toFIXED(player.z));
					slRotX(DEGtoANG(player.rx));  slRotZ(DEGtoANG(player.rz)); slRotY(DEGtoANG(player.ry));
					slRotX(DEGtoANG(player.rarm_rx)); slRotY(DEGtoANG(player.rarm_ry)); slRotZ(DEGtoANG(player.rarm_rz));
					jo_3d_set_scalef(0.6,0.6,0.6);	
					
					slPutPolygonX((XPDATA *)&xpdata_ham_RL1, light);
				}
				slPopMatrix();
				
				slPushMatrix();
				{
					slTranslate(toFIXED(player.x), toFIXED(player.y), toFIXED(player.z));
					slRotX(DEGtoANG(player.rx));  slRotZ(DEGtoANG(player.rz)); slRotY(DEGtoANG(player.ry));
					slRotX(DEGtoANG(player.lleg_rx)); slRotY(DEGtoANG(player.lleg_ry)); slRotZ(DEGtoANG(player.lleg_rz));
					jo_3d_set_scalef(0.6,0.6,0.6);	
					
					slPutPolygonX((XPDATA *)&xpdata_ham_LL2, light);
				}
				slPopMatrix();
				
				slPushMatrix();
				{
					slTranslate(toFIXED(player.x), toFIXED(player.y), toFIXED(player.z));
					slRotX(DEGtoANG(player.rx));  slRotZ(DEGtoANG(player.rz)); slRotY(DEGtoANG(player.ry));
					slRotX(DEGtoANG(player.rleg_rx)); slRotY(DEGtoANG(player.rleg_ry)); slRotZ(DEGtoANG(player.rleg_rz));
					jo_3d_set_scalef(0.6,0.6,0.6);	
					
					slPutPolygonX((XPDATA *)&xpdata_ham_RL2, light);
				}
				slPopMatrix();
				
				if(player.shadow_size >0)
				{
				slPushMatrix();
				{
					
					slTranslate(toFIXED(player.x), toFIXED(player.shadow_y), toFIXED(player.z));
					//slRotX(DEGtoANG(player.rx)); slRotY(DEGtoANG(player.ry)); slRotZ(DEGtoANG(player.rz));
					jo_3d_set_scalef(player.shadow_size,player.shadow_size,player.shadow_size);				
					slPutPolygon((PDATA *)&xpdata_ham_shadow);
				}
				slPopMatrix();
				}
				
				break;
				
		case 2:	
	
				slPushMatrix();
				{
					slTranslate(toFIXED(player.x), toFIXED(player.y), toFIXED(player.z));
					slRotX(DEGtoANG(player.rx)); slRotY(DEGtoANG(player.ry)); slRotZ(DEGtoANG(player.rz));
					slRotX(DEGtoANG(player.head_rx)); slRotY(DEGtoANG(player.head_ry)); slRotZ(DEGtoANG(player.head_rz));
					
					slPutPolygonX((XPDATA *)&xpdata_ham_head, light);
				}
				slPopMatrix();
				
				slPushMatrix();
				{
					slTranslate(toFIXED(player.x), toFIXED(player.y), toFIXED(player.z));
					slRotX(DEGtoANG(player.rx)); slRotY(DEGtoANG(player.ry)); slRotZ(DEGtoANG(player.rz));
					slRotX(DEGtoANG(player.body_rx)); slRotY(DEGtoANG(player.body_ry)); slRotZ(DEGtoANG(player.body_rz));
					
					slPutPolygonX((XPDATA *)&xpdata_ham_body, light);
				}
				slPopMatrix();
				
								
				slPushMatrix();
				{
					slTranslate(toFIXED(player.x), toFIXED(player.y), toFIXED(player.z));
					slRotX(DEGtoANG(player.rx));  slRotZ(DEGtoANG(player.rz)); slRotY(DEGtoANG(player.ry));
					slRotX(DEGtoANG(player.larm_rx)); slRotY(DEGtoANG(player.larm_ry)); slRotZ(DEGtoANG(player.larm_rz));
					
					slPutPolygonX((XPDATA *)&xpdata_ham_lwing, light);
				}
				slPopMatrix();
				
				slPushMatrix();
				{
					slTranslate(toFIXED(player.x), toFIXED(player.y), toFIXED(player.z));
					slRotX(DEGtoANG(player.rx));  slRotZ(DEGtoANG(player.rz)); slRotY(DEGtoANG(player.ry));
					slRotX(DEGtoANG(player.rarm_rx)); slRotY(DEGtoANG(player.rarm_ry)); slRotZ(DEGtoANG(player.rarm_rz));
					
					slPutPolygonX((XPDATA *)&xpdata_ham_rwing, light);
				}
				slPopMatrix();
				
				if(player.shadow_size >0)
				{
				slPushMatrix();
				{
					
					slTranslate(toFIXED(player.x), toFIXED(player.shadow_y), toFIXED(player.z));
					//slRotX(DEGtoANG(player.rx)); slRotY(DEGtoANG(player.ry)); slRotZ(DEGtoANG(player.rz));
					jo_3d_set_scalef(player.shadow_size,player.shadow_size,player.shadow_size);				
					slPutPolygon((PDATA *)&xpdata_ham_shadow);
				}
				slPopMatrix();
				}
				
				break;
				
		case 3:	
	
						
				slPushMatrix();
				{
					slTranslate(toFIXED(player.x), toFIXED(player.y), toFIXED(player.z));
					slRotX(DEGtoANG(player.rx)); slRotY(DEGtoANG(player.ry)); slRotZ(player.rz);
					slPutPolygonX((XPDATA *)&xpdata_ham_head,light);
				
					//lleg
					slPushMatrix();
					{
						slTranslate(toFIXED(8.0), toFIXED(0), toFIXED(0));
						slRotX(DEGtoANG(player.rleg_rx)); slRotY(DEGtoANG(player.rleg_ry)); slRotZ(DEGtoANG(player.rleg_rz));
						slPutPolygonX((XPDATA *)&xpdata_frog_LLeg1,light);
					
						slPushMatrix();
						{
							slTranslate(toFIXED(0.0), toFIXED(16), toFIXED(0));
							slRotX(DEGtoANG(0.0)); slRotY(DEGtoANG(0.0)); slRotZ(DEGtoANG(player.lleg_rz));
							slPutPolygonX((XPDATA *)&xpdata_frog_LLeg2,light);
						slPushMatrix();
							{
								slTranslate(toFIXED(0.0), toFIXED(16), toFIXED(0));
							slRotX(DEGtoANG(0.0)); slRotY(DEGtoANG(0.0)); slRotZ(DEGtoANG(0.0));
								slPutPolygonX((XPDATA *)&xpdata_frog_Leg3,light);
							}
							
							slPopMatrix();
							
						}	
						slPopMatrix();
						
					}
					slPopMatrix();
					//rleg
					slPushMatrix();
					{
						slTranslate(toFIXED(-8.0), toFIXED(0), toFIXED(0));
						slRotX(DEGtoANG(player.lleg_rx)); slRotY(DEGtoANG(player.lleg_ry)); slRotZ(DEGtoANG(-player.rleg_rz));
						slPutPolygonX((XPDATA *)&xpdata_frog_RLeg1,light);
					
						slPushMatrix();
						{
							slTranslate(toFIXED(0.0), toFIXED(16), toFIXED(0));
							slRotX(DEGtoANG(0.0)); slRotY(DEGtoANG(0.0)); slRotZ(DEGtoANG(-player.lleg_rz));
							slPutPolygonX((XPDATA *)&xpdata_frog_RLeg2,light);
						slPushMatrix();
							{
								slTranslate(toFIXED(0.0), toFIXED(16), toFIXED(0));
							slRotX(DEGtoANG(0.0)); slRotY(DEGtoANG(0.0)); slRotZ(DEGtoANG(0.0));
								slPutPolygonX((XPDATA *)&xpdata_frog_Leg3,light);
							}
							
							slPopMatrix();
							
						}	
						slPopMatrix();
						
					}
					slPopMatrix();
					
					}
				slPopMatrix();
				
				
				if(player.shadow_size >0)
				{
				slPushMatrix();
				{
					
					slTranslate(toFIXED(player.x), toFIXED(player.shadow_y), toFIXED(player.z));
					//slRotX(DEGtoANG(player.rx)); slRotY(DEGtoANG(player.ry)); slRotZ(DEGtoANG(player.rz));
					jo_3d_set_scalef(player.shadow_size,player.shadow_size,player.shadow_size);				
					slPutPolygon((PDATA *)&xpdata_ham_shadow);
				}
				slPopMatrix();
				}
				
				break;
				
		case 4:	
	
				slPushMatrix();
				{
					slTranslate(toFIXED(player.x), toFIXED(player.y), toFIXED(player.z));
					slRotX(DEGtoANG(player.rx)); slRotY(DEGtoANG(player.ry)); slRotZ(DEGtoANG(player.rz));
					slRotX(DEGtoANG(player.head_rx)); slRotY(DEGtoANG(player.head_ry)); slRotZ(DEGtoANG(player.head_rz));
					
					slPutPolygonX((XPDATA *)&xpdata_ham_head, light);
				}
				slPopMatrix();
				
						
								
				slPushMatrix();
				{
					slTranslate(toFIXED(player.x), toFIXED(player.y), toFIXED(player.z));
					slRotX(DEGtoANG(player.rx));  slRotZ(DEGtoANG(player.rz)); slRotY(DEGtoANG(player.ry));
					slRotX(DEGtoANG(player.larm_rx)); slRotY(DEGtoANG(player.larm_ry)); slRotZ(DEGtoANG(player.larm_rz));
					
					slPutPolygonX((XPDATA *)&xpdata_ham_dfin, light);
				}
				slPopMatrix();
				
				slPushMatrix();
				{
					slTranslate(toFIXED(player.x), toFIXED(player.y), toFIXED(player.z));
					slRotX(DEGtoANG(player.rx));  slRotZ(DEGtoANG(player.rz)); slRotY(DEGtoANG(player.ry));
					slRotX(DEGtoANG(player.rarm_rx)); slRotY(DEGtoANG(player.rarm_ry)); slRotZ(DEGtoANG(player.rarm_rz));
					
					slPutPolygonX((XPDATA *)&xpdata_ham_tfin, light);
				}
				slPopMatrix();
				
				if(player.shadow_size >0)
				{
				slPushMatrix();
				{
					
					slTranslate(toFIXED(player.x), toFIXED(player.shadow_y), toFIXED(player.z));
					//slRotX(DEGtoANG(player.rx)); slRotY(DEGtoANG(player.ry)); slRotZ(DEGtoANG(player.rz));
					jo_3d_set_scalef(player.shadow_size,player.shadow_size,player.shadow_size);				
					slPutPolygon((PDATA *)&xpdata_ham_shadow);
				}
				slPopMatrix();
				}
				
				//air bubbles
				if(player.left_cloud_size < 1.0f)
				{
					jo_sprite_enable_half_transparency();
					slPushMatrix();
					{
					slTranslate(toFIXED(player.left_cloud_x), toFIXED(player.left_cloud_y-8), toFIXED(player.left_cloud_z));
					jo_3d_set_scalef(player.left_cloud_size, player.left_cloud_size, player.left_cloud_size);
						if(player.left_cloud_y >= player.effect_y)
						{
						jo_3d_draw_billboard(8,0,0,0);//could replace with polygon as scale not working
						}
					}
					slPopMatrix();
					jo_sprite_disable_half_transparency();
					
				
				player.left_cloud_size += 0.005f;
				player.left_cloud_y -= 0.25f;
				}
				
				break;
		}
		
		
			if(player.effect_size <= 2.0f)
			{
				switch(player.effect_type)
				{
					
					case 0:
						slPushMatrix();
						{
							
							slTranslate(toFIXED(player.effect_x), toFIXED(player.effect_y), toFIXED(player.effect_z));
							jo_3d_set_scalef(player.effect_size,player.effect_size,player.effect_size);			
							slPutPolygonX((XPDATA *)&xpdata_splash_effect,light);
						}
						slPopMatrix();
						break;
						
					case 1:
						slPushMatrix();
						{
							
							slTranslate(toFIXED(player.x), toFIXED(player.y), toFIXED(player.z));
							//slRotY(DEGtoANG(player.ry));
							jo_3d_set_scalef(player.effect_size,player.effect_size,player.effect_size);			
							slPutPolygonX((XPDATA *)&xpdata_spin_effect,light);
						}
						slPopMatrix();
						break;
				
				}
			
			player.effect_size +=0.2f;	
			
			}
	
	slSetGouraudColor(CD_White);
	
	

	
	
}



void				map_builder_draw_section(void)
{
	
	slPushMatrix();
			{
				slTranslate(toFIXED(player.x), toFIXED(player.y), toFIXED(player.z));
				slRotX(DEGtoANG(player.rx)); slRotY(DEGtoANG(player.ry)); slRotZ(DEGtoANG(player.rz));
				jo_3d_set_scalef(object_scale,object_scale,object_scale);
				{	
					slPutPolygonX(xpdata_[map_builder_model], light);
				}
			}
			slPopMatrix();
	
}

void				map_builder_draw_enemy(void)
{

		switch(map_builder_enemy)
		{
			case 0:
		
			slPushMatrix();
			{
				slTranslate(toFIXED(player.x), toFIXED(player.y), toFIXED(player.z));
				slRotX(DEGtoANG(player.rx)); slRotY(DEGtoANG(player.ry)); slRotZ(DEGtoANG(player.rz));
				jo_3d_set_scalef(object_scale,object_scale,object_scale);
				//slPutPolygonX((XPDATA *)&xpdata_spider_body, light);
				slPutPolygonX((XPDATA *)enemy_data[map_builder_enemy], light);
				slPutPolygonX((XPDATA *)&xpdata_spider_LL1, light);
				slPutPolygonX((XPDATA *)&xpdata_spider_RL1, light);
				slPutPolygonX((XPDATA *)&xpdata_spider_LL2, light);
				slPutPolygonX((XPDATA *)&xpdata_spider_RL2, light);
				
			}
			slPopMatrix();
			
		
			
			break;
			
			case 1:
		
			slPushMatrix();
			{
				slTranslate(toFIXED(player.x), toFIXED(player.y), toFIXED(player.z));
				slRotX(DEGtoANG(player.rx)); slRotY(DEGtoANG(player.ry)); slRotZ(DEGtoANG(player.rz));
				jo_3d_set_scalef(object_scale,object_scale,object_scale);
				slPutPolygonX((XPDATA *)&xpdata_bat_head, light);
				//slPutPolygonX((XPDATA *)&xpdata_bat_body, light);
				slPutPolygonX((XPDATA *)enemy_data[map_builder_enemy], light);
				slPutPolygonX((XPDATA *)&xpdata_bat_lwing, light);
				slPutPolygonX((XPDATA *)&xpdata_bat_rwing, light);
			}
			slPopMatrix();
			
			
			break;
			
			case 2:	
	
						
				slPushMatrix();
				{
					slTranslate(toFIXED(player.x), toFIXED(player.y), toFIXED(player.z));
					slRotX(DEGtoANG(player.rx)); slRotY(DEGtoANG(player.ry)); slRotZ(DEGtoANG(player.rz));
					jo_3d_set_scalef(object_scale,object_scale,object_scale);
					//slPutPolygonX((XPDATA *)&xpdata_frog_body,light);
					slPutPolygonX((XPDATA *)enemy_data[map_builder_enemy], light);
				
					//lleg
					slPushMatrix();
					{
						slTranslate(toFIXED(8.0), toFIXED(0), toFIXED(0));
						slRotX(DEGtoANG(player.rleg_rx)); slRotY(DEGtoANG(player.rleg_ry)); slRotZ(DEGtoANG(-120));
						slPutPolygonX((XPDATA *)&xpdata_frog_LLeg1,light);
					
						slPushMatrix();
						{
							slTranslate(toFIXED(0.0), toFIXED(16), toFIXED(0));
							slRotX(DEGtoANG(0.0)); slRotY(DEGtoANG(0.0)); slRotZ(DEGtoANG(128));
							slPutPolygonX((XPDATA *)&xpdata_frog_LLeg2,light);
						slPushMatrix();
							{
								slTranslate(toFIXED(0.0), toFIXED(16), toFIXED(0));
							//slRotX(DEGtoANG(0.0)); slRotY(DEGtoANG(0.0)); slRotZ(DEGtoANG(0.0));
								slPutPolygonX((XPDATA *)&xpdata_frog_Leg3,light);
							}
							
							slPopMatrix();
							
						}	
						slPopMatrix();
						
					}
					slPopMatrix();
					//rleg
					slPushMatrix();
					{
						slTranslate(toFIXED(-8.0), toFIXED(0), toFIXED(0));
						slRotX(DEGtoANG(player.lleg_rx)); slRotY(DEGtoANG(player.lleg_ry)); slRotZ(DEGtoANG(120));
						slPutPolygonX((XPDATA *)&xpdata_frog_RLeg1,light);
					
						slPushMatrix();
						{
							slTranslate(toFIXED(0.0), toFIXED(16), toFIXED(0));
							slRotX(DEGtoANG(0.0)); slRotY(DEGtoANG(0.0)); slRotZ(DEGtoANG(-128));
							slPutPolygonX((XPDATA *)&xpdata_frog_RLeg2,light);
						slPushMatrix();
							{
								slTranslate(toFIXED(0.0), toFIXED(16), toFIXED(0));
							//slRotX(DEGtoANG(0.0)); slRotY(DEGtoANG(0.0)); slRotZ(DEGtoANG(0.0));
								slPutPolygonX((XPDATA *)&xpdata_frog_Leg3,light);
							}
							
							slPopMatrix();
							
						}	
						slPopMatrix();
						
					}
					slPopMatrix();
					
					}
				slPopMatrix();
				
				
				break;
				
			case 3:
		
			slPushMatrix();
			{
				slTranslate(toFIXED(player.x), toFIXED(player.y), toFIXED(player.z));
				slRotX(DEGtoANG(player.rx)); slRotY(DEGtoANG(player.ry)); slRotZ(DEGtoANG(player.rz));
				jo_3d_set_scalef(object_scale,object_scale,object_scale);
				//slPutPolygonX((XPDATA *)&xpdata_fish, light);
				slPutPolygonX((XPDATA *)enemy_data[map_builder_enemy], light);
				slPutPolygonX((XPDATA *)&xpdata_fish_dfin, light);
				slPutPolygonX((XPDATA *)&xpdata_fish_tfin, light);
				
			}
			slPopMatrix();
			
					
			break;
			
			
			
			
		}
	
}

void				map_builder_draw_powerup(void)
{
		slPushMatrix();
			{
				slTranslate(toFIXED(player.x), toFIXED(player.y), toFIXED(player.z));
				slRotX(DEGtoANG(player.rx)); slRotY(DEGtoANG(player.ry)); slRotZ(DEGtoANG(player.rz));
				jo_3d_set_scalef(object_scale,object_scale,object_scale);
				{
					slPutPolygonX((XPDATA *)pup_data[map_builder_powerup], light);
				}
			}
			slPopMatrix();
		
		
		
	
}

void				draw_map(level_section* section)
{
	int x_dist;
	int y_dist;
	int z_dist;
	
	if(player.in_water)
	{
	slSetGouraudColor(JO_COLOR_RGB(71,245,249));
	}else
	{
		slSetGouraudColor(CD_White);
	}
	
	//set draw distance
	x_dist = JO_ABS(player.x - section->x);
	y_dist = JO_ABS(player.y - section->y);
	z_dist = JO_ABS(player.z - section->z);
	
		
		if(x_dist < DRAW_DISTANCE_X)
		{
			if((x_dist + y_dist + z_dist) < DRAW_DISTANCE )
			{	
		
			if(section->type == 22)
			{
			replace_texture(section->map_model);
			}
			
			slPushMatrix();
					{
						slTranslate(toFIXED(section->x), toFIXED(section->y), toFIXED(section->z));
						{
							slPutPolygonX(section->map_model, light);
						}
					}
					slPopMatrix();
					
					//total_map_sections ++;
					
			}else if(((x_dist + y_dist + z_dist) < DRAW_DISTANCE_2) && section->map_model_lp->nbPolygon != 0 )
			{
					slPushMatrix();
					{
						slTranslate(toFIXED(section->x), toFIXED(section->y), toFIXED(section->z));
						{
							slPutPolygon(section->map_model_lp);
						}
					}
					slPopMatrix();	
					
					//total_map_sections ++;
			}
		}
			
}


/* 2 tiles */
static const jo_tile    HUD_Tileset[] =
{
	{0, 0, 48, 48},
	{48, 0, 48, 48},
	{96, 0, 32, 32}
	
};

void draw_hud(void)
{
		
		if(game.game_state == GAMESTATE_PAUSED)
		{
			
		jo_sprite_enable_half_transparency();
		//jo_sprite_change_sprite_scale(5);
		jo_sprite_change_sprite_scale_xy(11,8);
		jo_sprite_draw3D(0,0, 0, 100);
		jo_sprite_restore_sprite_scale();
		
		jo_sprite_disable_half_transparency();	
			
		}
		
	//jo_sprite_enable_half_transparency();
        switch(player.health) 	
		{
		case 1: jo_sprite_change_sprite_scale(0.3);
				break;
		case 2: jo_sprite_change_sprite_scale(0.6);
				break;
		case 3: jo_sprite_change_sprite_scale(1);
				break;
		}			

		
         jo_sprite_draw3D(player.health_sprite_id_1,-126, 86, 100);
		 
		jo_sprite_restore_sprite_scale();	
		 jo_sprite_draw3D(player.gems_sprite_id_1,-76, 74, 100);
		
		
		jo_nbg2_printf(14, 24,  "x%2d", player.gems);
		jo_nbg2_printf(17, 24,  "/25");
		if(cam_zoom_num != 4)
		{
		jo_nbg2_printf(8, 28,  "CAM ZOOM: %d   ",cam_zoom_num);
		}else
		{
		jo_nbg2_printf(8, 28,  "CAM ZOOM: AUTO");
		}
		
		//jo_sprite_draw3D(clock_sprite_id,-76, 100, 100);
		
		
       //jo_sprite_disable_half_transparency();
	   
	   
	
}



void			    my_draw(void)
{
	if (game.game_state != GAMESTATE_GAMEPLAY)
       return;
	if(show_debug)
	{
	jo_nbg2_printf(12, 0, "*ZYGO 3D*");
	}
    jo_3d_camera_look_at(&cam);
	Sint16 x_dist;
	Sint16 y_dist;
	Sint16 z_dist;
	slCurWindow(winFar);
	
	//animate_waterfall	
	animate_texture(MAP_TILESET+15,2,4);
	//jo_nbg2_printf(0, 12, "anim id: %d", anim_tex_num);
	 slPushMatrix();
    {
        if(use_light) computeLight();
     //   slTranslate(0, toFIXED(5.0), tz);
       // slRotX(DEGtoANG(rx));
		
    }
	slPopMatrix();
	
	//total_map_sections = 0;
	for(Uint16 i = 0; i < total_sections; i++)
	{
		draw_map(&map_section[i]);
	
	}
	
	
	draw_player();
	animate_player();
	player_collision_handling();
	
	for(Uint16 e = 0; e < enemy_total; e++)
	{	
	//set draw distance
	x_dist = JO_ABS(player.x - enemies[e].x);
	y_dist = JO_ABS(player.y - enemies[e].y);
	z_dist = JO_ABS(player.z - enemies[e].z);
	
		if((x_dist + y_dist + z_dist) < DRAW_DISTANCE_3)
		{		
			//if(enemies[e].alive)
			//{
			draw_enemies(&enemies[e]);
			animate_enemies(&enemies[e]);
			//}
		}
	}
	
	for(Uint16 p = 0; p < powerup_total; p++)
	{
	
	//set draw distance
	x_dist = JO_ABS(player.x - powerups[p].x);
	y_dist = JO_ABS(player.y - powerups[p].y);
	z_dist = JO_ABS(player.z - powerups[p].z);
	
		if((x_dist + y_dist + z_dist) < DRAW_DISTANCE_3)
		{
			//if(!powerups[p].used)
		//	{
			draw_powerups(&powerups[p]);
			//}
		}
	}
	slCurWindow(winNear);
	draw_hud();
   
   if(show_debug)
		{
		jo_nbg2_printf(0, 26, "POLYGON COUNT %4d" , jo_3d_get_polygon_count());
		jo_nbg2_printf(20, 1, "PPOS:\t%3d\t%3d\t%3d ",(int) player.x, (int) player.y, (int) player.z);
		jo_nbg2_printf(20, 2, "TOTAL MODELS   %d",model_total);
		jo_nbg2_printf(20, 3, "TOTAL SECTIONS %d",total_sections);
		jo_nbg2_printf(0, 27, "POLYGONS DISPLAYED %4d" , jo_3d_get_displayed_polygon_count());
		//jo_nbg2_printf(0, 2, "total sections: %3d",total_map_sections);
		//jo_nbg2_printf(0, 5, "* DYNAMIC MEMORY USAGE: %d%%  ", jo_memory_usage_percent());
	jo_nbg2_printf(0, 4,  "SPRITE MEMORY USAGE: %d%%  ", jo_sprite_usage_percent());
	//jo_nbg2_printf(0, 5,  "PLAYER.RY: %d%%  ", player.ry);
		}/*else
		{
		ztClearText();	
		}*/
    
}

void 			transition_to_level_select(void)
{
	ztClearText();
	jo_disable_background_3d_plane	(	JO_COLOR_Black	)	;
	jo_clear_background	(	JO_COLOR_Black	)	;
		//jo_clear_background(JO_COLOR_Black);
		//bg_sprite = jo_sprite_add_tga("BG", "LSEL.TGA", JO_COLOR_Transparent);
		//l_select_title = jo_sprite_add_tga("TEX", "SELT1.TGA", JO_COLOR_Red);
		//l_select_title2 = jo_sprite_add_tga("TEX", "SELT2.TGA", JO_COLOR_Red);
		//l_select_title3 = jo_sprite_add_tga("TEX", "SELT3.TGA", JO_COLOR_Red);
		//l_select_l1 = jo_sprite_add_tga("BG", "L1.TGA", JO_COLOR_Transparent);
		//l_select_l2 = jo_sprite_add_tga("BG", "L2.TGA", JO_COLOR_Transparent);
		//l_select_banner = jo_sprite_add_tga("TEX", "SELBAN.TGA", JO_COLOR_Transparent);
		game.game_state = GAMESTATE_LEVEL_SELECT;
}

void			clear_level(void)
{
	game.game_state = GAMESTATE_UNINITIALIZED;
	//clear_anim();		
	
	jo_sprite_free_from(game.map_sprite_id);
	jo_sprite_free_from(player.health_sprite_id_1);
	jo_sprite_free_from(player.gems_sprite_id_1);
	//jo_set_tga_palette_handling(JO_NULL);
				
	//enemy_total = 0;
	//powerup_total = 0;
	transition_to_level_select();
	
}

void			    end_level(void)
{
	if (game.game_state != GAMESTATE_END_LEVEL)
       return;
	
	jo_3d_camera_look_at(&cam);
	Sint16 x_dist;
	Sint16 y_dist;
	Sint16 z_dist;
	slCurWindow(winFar);
	
	//animate_waterfall	
	animate_texture(32,3,4);
	//jo_nbg2_printf(0, 12, "anim id: %d", anim_tex_num);
	 slPushMatrix();
    {
        if(use_light) computeLight();
     //   slTranslate(0, toFIXED(5.0), tz);
       // slRotX(DEGtoANG(rx));
		
    }
	slPopMatrix();
	
	
	for(Uint16 i = 0; i < total_sections; i++)
	{
		draw_map(&map_section[i]);
	
	}
	
	
	draw_player();
	//animate_player();
	//player_collision_handling();
	
	player.ry++;
	
	for(Uint16 e = 0; e < enemy_total; e++)
	{	
	//set draw distance
	x_dist = JO_ABS(player.x - enemies[e].x);
	y_dist = JO_ABS(player.y - enemies[e].y);
	z_dist = JO_ABS(player.z - enemies[e].z);
	
		if((x_dist + y_dist + z_dist) < DRAW_DISTANCE)
		{		
			if(enemies[e].alive)
			{
			draw_enemies(&enemies[e]);
			//animate_enemies(&enemies[e]);
			}
		}
	}
	
	for(Uint16 p = 0; p < powerup_total; p++)
	{
	
	//set draw distance
	x_dist = JO_ABS(player.x - powerups[p].x);
	y_dist = JO_ABS(player.y - powerups[p].y);
	z_dist = JO_ABS(player.z - powerups[p].z);
	
		if((x_dist + y_dist + z_dist) < DRAW_DISTANCE)
		{
			//if(!powerups[p].used)
		//	{
			draw_powerups(&powerups[p]);
			//}
		}
	}
	
	jo_nbg2_printf(0, 1, "                *LEVEL COMPLETE*");

  //  slCurWindow(winNear);
	//draw_hud();
	
	
	jo_nbg2_printf(10, 12, "RESTART LEVEL");
    jo_nbg2_printf(10, 13, "QUIT TO TITLE");
    

		
	if (KEY_DOWN(0,PER_DGT_KU))
		
    {
            jo_nbg2_printf(9, 12, " ");
            jo_nbg2_printf(9, 13, " ");
            
			
            if (game.end_level_menu == 0)
                game.end_level_menu = END_LEVEL_MENU_MAX-1;
            else
                game.end_level_menu --;



            


        
    }
    

   if (KEY_DOWN(0,PER_DGT_KD))
    {
        
            jo_nbg2_printf(9, 12, " ");
            jo_nbg2_printf(9, 13, " ");
            
			

            if (game.end_level_menu == END_LEVEL_MENU_MAX-1)
                game.end_level_menu = 0;
            else
                game.end_level_menu ++;



            
        
    }
	
	jo_nbg2_printf(9, 12 + game.end_level_menu, "*");
    

    // did player one pause the game?
   if (KEY_DOWN(0,PER_DGT_ST))
    {
            ztClearText();
            if (game.end_level_menu == 0)
            {
                game.game_state = GAMESTATE_GAMEPLAY;
				
                reset_demo();
				//jo_enable_background_3d_plane	(	JO_COLOR_Black	)	;
				//num_players = game.players;
            }

            else
            {
				
				game.game_state = GAMESTATE_UNINITIALIZED;
				clear_level();
				//jo_disable_background_3d_plane	(	JO_COLOR_Black	)	;
				//jo_clear_background	(	JO_COLOR_Black	)	;
			 }
			 
            


        
    }
   
	
   
  
    
}

void			    object_viewer(void)
{
	if (game.game_state != GAMESTATE_OBJECT_VIEWER)
		
       return;
   
	XPDATA * current_object;
	Uint32 nbPt;
	
	jo_nbg2_printf(0, 1, "                *OBJECT VIEWER");
	jo_nbg2_printf(0, 2, "MODEL TYPE: %2d", map_builder_mode);
	jo_nbg2_printf(0, 3, "POLYGON: %3d", object_pol_num);
	jo_nbg2_printf(20, 2, "MODEL NUMBER: %2d", object_number);
	jo_nbg2_printf(0, 28, "Y TO CHANGE TYPE, B TO CHANGE OBJ NUM,  ");
	jo_nbg2_printf(0, 29, "C TO SHOW POLYGON, A TO CHANGE POLYGON  ");
	
   
   jo_3d_camera_look_at(&cam);
	
   switch(map_builder_mode)
			{
			case 0:
					object_number = map_builder_model;	
					current_object= xpdata_[map_builder_model];
						
					break;
			
			case 1:
					object_number = map_builder_enemy;	
					current_object=(XPDATA *)enemy_data[map_builder_enemy];//need to add struct for enemy models					
					break;
					
			case 2:
					object_number = map_builder_powerup;	
					current_object=(XPDATA *)pup_data[map_builder_powerup];				
					break;
			}
   
	nbPt = current_object->nbPolygon;
   
   
   switch(map_builder_mode)
		{
        case 0:
				map_builder_draw_section();
				break;
				
		case 1:
				map_builder_draw_enemy();
				break;
				
		case 2:
				map_builder_draw_powerup();
				break;		
		
		}
   
   
   delta_x = 0;
	delta_y = 0.0f;
	delta_z = 0;
	
	if (!jo_is_pad1_available())
		return;
	
	if (KEY_PRESS(0,PER_DGT_KU))
		{
		//rotate x	
		player.rx--;
		}
			
	if (KEY_PRESS(0,PER_DGT_KD))
		{
		//rotate -x	
		player.rx++;		
		}
	
	if (KEY_PRESS(0,PER_DGT_KL))
		{
		//rotate -y	
		player.ry--;
		}
		
	if (KEY_PRESS(0,PER_DGT_KR))
		{//rotate y		
		player.ry++;
		}	
		
		
	if (KEY_PRESS(0,PER_DGT_TL))
		{//zoom out			
		if(object_scale >0.1f)
			object_scale-=0.05f;
		}
		
	if (KEY_PRESS(0,PER_DGT_TR))
		{
		//zoom in	
		if(object_scale <2.0f)
			object_scale+=0.05f;		
		}
		
	if (KEY_DOWN(0,PER_DGT_TB))
		{
			
			if(object_show_poly)
					{
						current_object->attbl[object_pol_num].texno = object_last_texture;
						current_object->attbl[object_pol_num].colno = LUTidx(object_last_texture);
						object_show_poly = false;
					}
			object_pol_num = 0;
			
			switch(map_builder_mode)
			{
			case 0:	++map_builder_model;
					if (map_builder_model >= model_total)
					{
					map_builder_model = 0;	
					}  
					break;
			
			case 1:	++map_builder_enemy;
					if (map_builder_enemy >= 4)
					{
					map_builder_enemy = 0;			
					}  
					break;
					
			case 2:	++map_builder_powerup;
					if (map_builder_powerup >= 7)
					{
					map_builder_powerup = 0;			
					}  
					break;
			}
			      
		}
	
	if (KEY_DOWN(0,PER_DGT_TY))
		{
			
			if(object_show_poly)
					{
						current_object->attbl[object_pol_num].texno = object_last_texture;
						current_object->attbl[object_pol_num].colno = LUTidx(object_last_texture);
						object_show_poly = false;
					}
			object_pol_num = 0;
			
			if(map_builder_mode >=2)
			{
			map_builder_mode=0;
			}else
			{
			map_builder_mode++;
			}
		}
		
	if (KEY_DOWN(0,PER_DGT_TA) && object_show_poly)
		{
			object_last_pol_num = object_pol_num;
			
			if(object_pol_num == nbPt-1)
			{
			object_pol_num = 0;
			}else
			{
			object_pol_num++;
			}
			
			current_object->attbl[object_last_pol_num].texno = object_last_texture;
			current_object->attbl[object_last_pol_num].colno = LUTidx(object_last_texture);
			object_last_texture = (Uint16) current_object->attbl[object_pol_num].texno;
			current_object->attbl[object_pol_num].texno = MAP_TILESET+7;
			current_object->attbl[object_pol_num].colno = LUTidx(MAP_TILESET+7);
			
		}
		
	if (KEY_DOWN(0,PER_DGT_TC))
		{
			if(object_show_poly)
			{
				current_object->attbl[object_pol_num].texno = object_last_texture;
				current_object->attbl[object_pol_num].colno = LUTidx(object_last_texture);
				object_show_poly = false;
			}else
			{
				object_show_poly = true;
				object_pol_num = 0;
				object_last_texture = (Uint16) current_object->attbl[object_pol_num].texno;
				object_last_pol_num = object_pol_num;
				current_object->attbl[object_pol_num].texno = MAP_TILESET+7;
				current_object->attbl[object_pol_num].colno = LUTidx(MAP_TILESET+7);
			}
		}
	
	 // did player one pause the game?
    if (KEY_DOWN(0,PER_DGT_ST))
		{
			if(game.pressed_start == false)
			{
				player.rx = 0;player.ry=0;player.rz=0;
				jo_nbg2_printf(0, 28, "                                       ");
				jo_nbg2_printf(0, 29, "                ");
				game.game_state = GAMESTATE_PAUSED;
				game.pause_menu = 0;
				jo_nbg2_printf(9, 12, "*");
			}
			game.pressed_start = true;
		}
    else
		{
			game.pressed_start = false;
		}
		
		jo_nbg2_printf(0, 4, "POLYS: %3d", jo_3d_get_polygon_count());
    jo_nbg2_printf(20, 4, "VERTS: %3d", jo_3d_get_vertices_count());
		
}

void			    map_builder(void)
{
	if (game.game_state != GAMESTATE_MAP_BUILDER)
		
       return;
    
	int i;
	bool collide;
	int snap;// 0 for no snap, 1 for snap left, 2 for snap right
	int x_dist;
	int y_dist;
	int z_dist;
	int total_poly;
	int prev_x;
	int prev_y;
	int prev_z;
	
	
						
   player.speed_y = 0.0f;
			delta_y = 0.0f;
			
			
    jo_nbg2_printf(0, 1, "                *MAP BUILDER");
	jo_nbg2_printf(0, 1, "SECTIONS: %d", total_sections);
	//jo_nbg2_printf(0, 3, "ENEMIES : %d, Powerups %d", enemy_total, powerup_total);
	jo_nbg2_printf(20, 27, "GRIDSIZE   %3d",gridsize);
	

    jo_3d_camera_look_at(&cam);
	
	for(i = 0; i < total_sections; i++)
	{
	draw_map(&map_section[i]);
	}
	
	for(int e = 0; e < enemy_total; e++)
	{	
	//set draw distance
	x_dist = JO_ABS(player.x - enemies[e].x);
	y_dist = JO_ABS(player.y - enemies[e].y);
	z_dist = JO_ABS(player.z - enemies[e].z);
	
		if((x_dist + y_dist + z_dist) < DRAW_DISTANCE)
		{		
			
			draw_enemies(&enemies[e]);
			animate_enemies(&enemies[e]);
			
		}
	}
	
	for(int p = 0; p < powerup_total; p++)
	{
	
	//set draw distance
	x_dist = JO_ABS(player.x - powerups[p].x);
	y_dist = JO_ABS(player.y - powerups[p].y);
	z_dist = JO_ABS(player.z - powerups[p].z);
	
		if((x_dist + y_dist + z_dist) < DRAW_DISTANCE)
		{
			//if(!powerups[p].used)
		//	{
			draw_powerups(&powerups[p]);
			//}
		}
	}
	
	//snap
	if(map_builder_model == 0)
	{
		if(player.x + 128 == map_section[total_sections-1].x && player.y == map_section[total_sections-1].y && player.z == map_section[total_sections-1].z)
		{
			if(map_section[total_sections-1].type == 0)
			{snap = 1;
			}else if (map_section[total_sections-1].type == 25)
			{snap = 3;
			}
		}
		else if(player.x - 128 == map_section[total_sections-1].x && player.y == map_section[total_sections-1].y && player.z == map_section[total_sections-1].z)
		{
			if(map_section[total_sections-1].type == 0)
			{snap = 2;
			}else if (map_section[total_sections-1].type == 27)
			{snap = 4;
			}
		}
		else
		{
		snap = 0;
		}
	
	}else
		{
		snap = 0;
		}
	
	if (!map_builder_delete_mode)
	{
		jo_nbg2_printf(0, 27, "ADD   %d",map_builder_mode);
		jo_nbg2_printf(0, 28, "A TO ADD SECTION, B TO CHANGE SECTION,  ");
		jo_nbg2_printf(0, 29, "Z FOR DELETE MODE, Y TO CHANGE TYPE     ");
		
		switch(map_builder_mode)
		{
        case 0:
				for(i = 0; i < total_sections; i++)
				{
				//set map section distance	
				x_dist = JO_ABS(player.x - map_section[i].x);
				y_dist = JO_ABS(player.y - map_section[i].y);
				z_dist = JO_ABS(player.z - map_section[i].z);
	
				section_dist = x_dist + y_dist + z_dist;
				
				collide = has_map_collision(&map_section[i],map_builder_model, player.x, player.y, player.z);
				
					if(collide)
					{
					slSetGouraudColor(CD_Red);
					break;
					}else
					{
						if(snap != 0)
						{
						slSetGouraudColor(CD_Blue);
						}else
						{
						slSetGouraudColor(CD_White);
						}
					}
				}
				map_builder_draw_section();
				
				break;
				
		case 1:
				
				map_builder_draw_enemy();
				break;
				
		case 2:
				
				map_builder_draw_powerup();
				break;		
		
		}
		
		
	}
	else
	{
		jo_nbg2_printf(0, 27, "DELETE");
		jo_nbg2_printf(0, 28, "A TO ADD SECTION, B TO CHANGE SECTION,  ");
		jo_nbg2_printf(0, 29, "Z FOR ADD MODE, Y TO CHANGE TYPE        ");
	}
	
	
	
	
      
    //jo_nbg2_printf(0, 28, "Polygon count: %d  ", jo_3d_get_polygon_count());
	
	jo_nbg2_printf(0, 2, "MAP POSITION: %7d %7d %7d  ", player.x, player.y, player.z);
	jo_nbg2_printf(0, 3, "LASTPOSITION: %7d %7d %7d  ", map_section[total_sections-1].x, map_section[total_sections-1].y, map_section[total_sections-1].z);
	
	delta_x = 0;
	delta_y = 0.0f;
	delta_z = 0;
	
	if (!jo_is_pad1_available())
		return;
	
	if (KEY_DOWN(0,PER_DGT_KU))
		{
		player.z += gridsize;
		delta_z += JO_DIV_BY_2(gridsize);	
		}
			
	if (KEY_DOWN(0,PER_DGT_KD))
		{
		player.z -= gridsize;
		delta_z -= JO_DIV_BY_2(gridsize);			
		}
	
	if (KEY_DOWN(0,PER_DGT_KL))
		{
		player.x -= gridsize;
		delta_x -= JO_DIV_BY_2(gridsize);			
		}
		
	if (KEY_DOWN(0,PER_DGT_KR))
		{player.x += gridsize;
		delta_x += JO_DIV_BY_2(gridsize);			
		}	
		
		
	if (KEY_DOWN(0,PER_DGT_TL))
		{player.y -= gridsize;
		delta_y -= JO_DIV_BY_2(gridsize);			
		}
		
	if (KEY_DOWN(0,PER_DGT_TR))
		{
		player.y += gridsize;
		delta_y += JO_DIV_BY_2(gridsize);			
		}
		
	total_poly = jo_3d_get_polygon_count();
		
	if (KEY_DOWN(0,PER_DGT_TA) && !map_builder_delete_mode && total_poly <500)
		{
			switch(map_builder_mode)
			{
			case 0:
					if(!collide && total_sections <249)
					{
						prev_x = map_section[total_sections-1].x;
						prev_y = map_section[total_sections-1].y;
						prev_z = map_section[total_sections-1].z;
						if(snap == 1)
						{
						total_sections--;
						create_map_section(&map_section[total_sections], 27,prev_x,prev_y,prev_z);total_sections++; 
						create_map_section(&map_section[total_sections], 25,player.x,player.y,player.z);total_sections++; 
						}
						else if(snap == 2)
						{
						total_sections--;
						create_map_section(&map_section[total_sections], 25,prev_x,prev_y,prev_z);total_sections++; 
						create_map_section(&map_section[total_sections], 27,player.x,player.y,player.z);total_sections++; 
						}else if(snap == 3)
						{
						total_sections--;
						create_map_section(&map_section[total_sections], 26,prev_x,prev_y,prev_z);total_sections++; 
						create_map_section(&map_section[total_sections], 25,player.x,player.y,player.z);total_sections++; 
						}else if(snap == 4)
						{
						total_sections--;
						create_map_section(&map_section[total_sections], 26,prev_x,prev_y,prev_z);total_sections++; 
						create_map_section(&map_section[total_sections], 27,player.x,player.y,player.z);total_sections++; 
						}else
						{
						create_map_section(&map_section[total_sections], map_builder_model,player.x,player.y,player.z);total_sections++; 
						}
					
					}
					break;
			
			case 1:	if(enemy_total <30)
					{
					create_enemy(&enemies[enemy_total],map_builder_enemy+1,player.x,player.y,player.z,64,64,4,1);enemy_total++;
					}
					break;
					
			case 2:
					if(powerup_total <30)
					{
					create_powerup(&powerups[powerup_total],map_builder_powerup+1,player.x,player.y,player.z);powerup_total++;
					}
					break;
		
		
			}
		}
	
	if (KEY_DOWN(0,PER_DGT_TA) && map_builder_delete_mode)
	
		{
        switch(map_builder_mode)
			{
			case 0:
					if(total_sections > 0)
					total_sections--; 
					break;
			
			case 1:
					if(enemy_total > 0)
					enemy_total--; 
					break;
					
			case 2:
					if(powerup_total > 0)
					powerup_total--; 
					break;
			}
		}
		
	if (KEY_DOWN(0,PER_DGT_TB))
		{
			switch(map_builder_mode)
			{
			case 0:
					++map_builder_model;
					if (map_builder_model >= model_total)
					{
					map_builder_model = 0;			
					}  
					break;
			
			case 1:
					++map_builder_enemy;
					if (map_builder_enemy >= 4)
					{
					map_builder_enemy = 0;			
					}  
					break;
					
			case 2:
					++map_builder_powerup;
					if (map_builder_powerup >= 7)
					{
					map_builder_powerup = 0;			
					}  
					break;
			}
			      
		}
		
		if (KEY_DOWN(0,PER_DGT_TC))
		{
			//change grid size (default 32)
			if(gridsize < 128)
			{
			gridsize = gridsize *2;
			}else
			{
			gridsize = 8;
			}
			
			
		}
		
		if (KEY_DOWN(0,PER_DGT_TX))
		 {	
			if(cam_zoom_num == 4)
			{
			cam_zoom_num = 0;
			}else
			{
			 cam_zoom_num ++;
			}
		 }
	
	if (KEY_DOWN(0,PER_DGT_TZ))
		{
        map_builder_delete_mode ^= true;
		}
		
	if (KEY_DOWN(0,PER_DGT_TY))
		{
			if(map_builder_mode >=2)
			{
			map_builder_mode=0;
			}else
			{
			map_builder_mode++;
			}
		}
		

	
	 // did player one pause the game?
    if (KEY_DOWN(0,PER_DGT_ST))
		{
			if(game.pressed_start == false)
			{
				
				jo_nbg2_printf(0, 28, "                                       ");
				jo_nbg2_printf(0, 29, "                ");
				game.game_state = GAMESTATE_PAUSED;
				game.pause_menu = 0;
				jo_nbg2_printf(9, 12, "*");
			}
			game.pressed_start = true;
		}
    else
		{
			game.pressed_start = false;
		}
		
	player.y += player.speed_y;
	delta_y += (player.speed_y);
	
	cam_pos_x += (delta_x*jo_cos(cam_angle_y) + delta_y*jo_sin(cam_angle_y))/32768;
	cam_pos_y = player.y + cam_height;
	cam_pos_z += ((delta_z*jo_cos(cam_angle_y) + delta_x*jo_sin(cam_angle_y))/32768);
	
	cam_target_x = player.x;
	cam_target_y = player.y;
	cam_target_z = player.z;

	jo_nbg2_printf(0, 4, "POLYGON COUNT: %d  ", total_poly);
	
	slSetGouraudColor(CD_White);
}

void			    pause_game(void)
{
	if (game.game_state != GAMESTATE_PAUSED)
       return;
   
   
   //draw screen still
   
   jo_3d_camera_look_at(&cam);
 
	
	for(int i = 0; i < total_sections; i++)
	{
	draw_map(&map_section[i]);
	}
	
	draw_player();
   
   ///
   
    jo_nbg2_printf(16, 8, "*PAUSED*");

    slCurWindow(winNear);
	draw_hud();
	
	
	jo_nbg2_printf(10, 12, "CONTINUE");
    jo_nbg2_printf(10, 13, "RESTART LEVEL");
    jo_nbg2_printf(10, 14, "MAP BUILDER");
	jo_nbg2_printf(10, 15, "QUIT LEVEL SELECT");
	
		
	if (KEY_DOWN(0,PER_DGT_KU))
		
    {
        if(player.pressed_up == false)
        {

            jo_nbg2_printf(9, 12, " ");
            jo_nbg2_printf(9, 13, " ");
            jo_nbg2_printf(9, 14, " ");
			jo_nbg2_printf(9, 15, " ");
			
            if (game.pause_menu == 0)
                game.pause_menu = PAUSE_MENU_MAX;
            else
                game.pause_menu --;



            jo_nbg2_printf(9, 12 + game.pause_menu, "*");


        }
        player.pressed_up = true;
    }
    else
    {
        player.pressed_up = false;
    }

   if (KEY_DOWN(0,PER_DGT_KD))
    {
        if(player.pressed_down == false)
        {
            jo_nbg2_printf(9, 12, " ");
            jo_nbg2_printf(9, 13, " ");
            jo_nbg2_printf(9, 14, " ");
			jo_nbg2_printf(9, 15, " ");
			

            if (game.pause_menu == PAUSE_MENU_MAX)
                game.pause_menu = 0;
            else
                game.pause_menu ++;



            jo_nbg2_printf(9, 12 + game.pause_menu, "*");
        }
        player.pressed_down = true;
    }
    else
    {
        player.pressed_down = false;
    }

    // did player one pause the game?
   if (KEY_DOWN(0,PER_DGT_ST))
    {
        if(game.pressed_start == false)
        {
            ztClearText();
            if (game.pause_menu == 1)
            {
                game.game_state = GAMESTATE_GAMEPLAY;
				
                reset_demo();
				//jo_enable_background_3d_plane	(	JO_COLOR_Black	)	;
				//num_players = game.players;
            }

            else if (game.pause_menu == 2)
            {
				//reset_demo();
				player.x = roundUp(player.x,32);
				player.y = roundUp(player.y,32);
				player.z = roundUp(player.z,32);
				
                game.game_state = GAMESTATE_MAP_BUILDER;
				//jo_disable_background_3d_plane	(	JO_COLOR_Black	)	;
				//jo_clear_background	(	JO_COLOR_Black	)	;
			 }
			 else if (game.pause_menu == 3)
            {
							
                game.game_state = GAMESTATE_UNINITIALIZED;
				clear_level();
			 }
			 
            else
			{game.game_state = GAMESTATE_GAMEPLAY;
			
			}


        }
        game.pressed_start = true;
    }
    else
    {
        game.pressed_start = false;
    }
	
		slCurWindow(winFar);
}

void				move_camera(void)

{
					
	switch(cam_number)
		{
        case 1:
				next_cam_x = 0;
				next_cam_z = - CAM_DIST_DEFAULT;
				break;
				
		case 2:
				next_cam_x = CAM_DIST_DEFAULT;
				next_cam_z = 0;
				break;
		
		case 3:
				next_cam_x = 0;
				next_cam_z = CAM_DIST_DEFAULT;
				break;
				
		case 4:
				next_cam_x = - CAM_DIST_DEFAULT;
				next_cam_z = 0;
				break;
				
		default:
				next_cam_x = 0;
				next_cam_z = - CAM_DIST_DEFAULT;
				break;
		}
		
		if (cam_adj_x != next_cam_x)
		{
			if (next_cam_x > cam_adj_x)
			{
				cam_adj_x += CAM_SPEED;
			}
			else
			{
				cam_adj_x -= CAM_SPEED;
			}
		}
		
		if (cam_adj_z != next_cam_z)
		{
			if (next_cam_z > cam_adj_z)
			{
				cam_adj_z += CAM_SPEED;
			}
			else
			{
				cam_adj_z -= CAM_SPEED;
			}
		}
		
		//camera zoom
		int y_dist;
		
		
		switch(cam_zoom_num)
		{
		case 0:
				cam_zoom = CAM_DEFAULT_Y;
				break;
        case 1:
				cam_zoom = CAM_DEFAULT_Y - CAM_ZOOM_1;
				break;
				
		case 2:
				cam_zoom = CAM_DEFAULT_Y - CAM_ZOOM_2;
				break;
				
		case 3:
				cam_zoom = CAM_DEFAULT_Y - CAM_ZOOM_3;
				break;
				
		case 4:
				cam_zoom = CAM_DEFAULT_Y + roundUp(JO_DIV_BY_2(player.y),CAM_SPEED);
		}
		
		//if(cam_zoom_num != 4)
		//{
		y_dist = cam_height - cam_zoom;
		
		if(y_dist >0)
		{
			cam_height -= CAM_SPEED;
		}else if(y_dist <0)
		{
			cam_height += CAM_SPEED;
		}
		//}else
		//{
		//cam_height = cam_zoom;	
		//}
				
		
	jo_3d_camera_set_viewpoint(&cam,cam_pos_x,cam_pos_y,cam_pos_z);
	jo_3d_camera_set_target(&cam,cam_target_x,cam_target_y,cam_target_z);
	rot.rz = JO_DEG_TO_RAD(jo_atan2f(-(cam_target_x - cam_pos_x),-(cam_target_z - cam_pos_z)));
	//rot.rx =  JO_DEG_TO_RAD(cam_target_y - cam_pos_y) +90;
	rot.rx = JO_DEG_TO_RAD(-jo_atan2f(-(cam_target_z - cam_pos_z),-(cam_target_y - cam_pos_y)));
	//cam_angle_x = ((cam_pos_x*32768 * jo_sin(cam_angle_y))/32768);
	//cam_angle_z = ((cam_pos_z*32768 * jo_cos(cam_angle_y))/32768);
	
	
}





void            title_screen(void)
{
	
	if (game.game_state != GAMESTATE_TITLE_SCREEN)
    return;
	
	jo_3d_camera_look_at(&cam);
	jo_3d_camera_set_viewpoint(&cam,0,0,0);
	jo_3d_camera_set_target(&cam,map_section[0].x,map_section[0].y,map_section[0].z);
	
	slPushMatrix();
    {
        if(use_light) computeLight();
     //   slTranslate(0, toFIXED(5.0), tz);
       // slRotX(DEGtoANG(rx));
		
    }
	slPopMatrix();
	
 //jo_nbg2_printf(0, 1, "               *ZYGO 3D*");
jo_nbg2_printf(14, 26, "START GAME");

	/*for(Uint16 i = 0; i < total_sections; i++)
	{
		draw_map(&map_section[i]);
	
	}*/

	
	slPushMatrix();
				{
					slTranslate(toFIXED(map_section[0].x), toFIXED(map_section[0].y), toFIXED(map_section[0].z+100));
					///slRotX(DEGtoANG(player.rx)); slRotY(DEGtoANG(player.ry)); slRotZ(DEGtoANG(player.rz));
					slRotX(DEGtoANG(player.head_rx)); slRotY(DEGtoANG(player.head_ry)); slRotZ(DEGtoANG(player.head_rz));
					//jo_3d_set_scalef(4,4,4);	
					
					slPutPolygonX(map_section[0].map_model, light);
				}
				slPopMatrix();
				
	player.head_ry ++;
	
	
	if (KEY_DOWN(0,PER_DGT_ST))
	 {	
		game.game_state = GAMESTATE_UNINITIALIZED;
		ztClearText();
		game.pressed_start = true;
		transition_to_level_select();
	 }
	 
	
	
}


void my_vblank()
{

    if(enableRTG == 1)
        slGouraudTblCopy();
}

void init_display(void)
{
	
	//slInitSystem(TV_320x240, NULL, 1);
	jo_core_init(JO_COLOR_Black);
    //jo_core_init(JO_COLOR_RGB(98, 215, 217));
	//jo_core_init(JO_COLOR_RGB(151, 138, 163));
	
    jo_3d_camera_init(&cam);
	slCurWindow(winFar);
	jo_3d_window(0, 0, JO_TV_WIDTH-1, JO_TV_HEIGHT-1, DRAW_DISTANCE_MAX, JO_TV_WIDTH_2, JO_TV_HEIGHT_2); //Includes the draw distance. Also I left 40 pixels for a HUD.
	
	slSetDepthLimit(0,10,5);//slSetDepthLimit(0,dist,5);
	slSetDepthTbl(DepthDataBlack,0xf000,32);
	
	slInitGouraud(gourRealMax, GOUR_REAL_MAX, GRaddr, vwork);
	slIntFunction(my_vblank);
	/**Set your color here if you need one (depending on your light source)**/
    slSetGouraudColor(CD_White);
	
	
	slCurWindow(winNear);
	slWindow(0, 0, JO_TV_WIDTH-1, JO_TV_HEIGHT-1, DRAW_DISTANCE, JO_TV_WIDTH_2, JO_TV_HEIGHT_2); //Includes the draw distance. Also I left 40 pixels for a HUD.
	
	
	jo_core_set_screens_order(JO_NBG2_SCREEN, JO_SPRITE_SCREEN, JO_RBG0_SCREEN);
	//slPriorityNbg3(7);
		
}




void			my_gamepad(void)
{
	if (game.game_state != GAMESTATE_GAMEPLAY)
       return;
   
	player.delta_x = 0.0f;
	player.delta_y = 0.0f;
	player.delta_z = 0.0f;
	float player_speed;
	
	if(player.in_water && player.type != 4)
	{
	player_speed = player.speed/2;	
		
	}else
	{
	player_speed = player.speed;	
	}
	if (KEY_PRESS(0,PER_DGT_KU) && KEY_PRESS(0,PER_DGT_KL))
			{player.dpad = 8;}//up left
			else if (KEY_PRESS(0,PER_DGT_KU) && KEY_PRESS(0,PER_DGT_KR))
			{player.dpad = 2;}//up right
			else if (KEY_PRESS(0,PER_DGT_KD) && KEY_PRESS(0,PER_DGT_KL))
			{player.dpad = 6;}//down left
			else if (KEY_PRESS(0,PER_DGT_KD) && KEY_PRESS(0,PER_DGT_KR))
			{player.dpad = 4;}//down right
			else if (KEY_PRESS(0,PER_DGT_KU))
			{player.dpad = 1;}//up
			else if (KEY_PRESS(0,PER_DGT_KD))
			{player.dpad = 5;}//down
			else if (KEY_PRESS(0,PER_DGT_KL))
			{player.dpad = 7;}//left
			else if (KEY_PRESS(0,PER_DGT_KR))
			{player.dpad = 3;}//right
			else
			{player.dpad = 0;}//nothing pressed
	
	
	if (KEY_DOWN(0,PER_DGT_TL) && show_debug)
			{
			if(cam_number <4)
				cam_number +=1;
			else
				cam_number = 1;
			}
		
		if (KEY_DOWN(0,PER_DGT_TR) && show_debug)	
			{
			if(cam_number > 1)
				cam_number -=1;
			else
				cam_number = 4;
			}
	
	
	
	if(player.dpad == 8)
		{
			switch(cam_number)
			{
			case 1: 
					player.speed_z = player_speed/2;//up left
					player.speed_x = -player_speed/2;
					if(!player.special_attack)
					{player.ry = 135;}
					break;
			case 2: 
					player.speed_z = -player_speed/2;//down left
					player.speed_x = -player_speed/2;
					player.ry = 45;
					break;
			case 3: 
					player.speed_z = -player_speed/2;//down right
					player.speed_x = player_speed/2;
					player.ry = -45;
					break;
			case 4: 
					player.speed_z = player_speed/2;// up right
					player.speed_x = player_speed/2;
					player.ry = -135;
					break;
			}
		}else if(player.dpad == 2)
		{
			switch(cam_number)
			{
			
			case 1: 
					player.speed_z = player_speed/2; //up right
					player.speed_x = player_speed/2;
					if(!player.special_attack)
					{player.ry = -135;}
					break;
			case 2: 
					player.speed_z = player_speed/2;//up left
					player.speed_x = -player_speed/2;
					player.ry = 135;
					break;
			case 3: 
					player.speed_z = -player_speed/2;//down left
					player.speed_x = -player_speed/2;
					player.ry = 45;
					break;
			case 4: 
					player.speed_z = -player_speed/2;//down right
					player.speed_x = player_speed/2;
					player.ry = -45;
					break;
			
			}
		}else if(player.dpad == 4)
		{
			switch(cam_number)
			{
			case 1: 
					player.speed_z = -player_speed/2;//down right
					player.speed_x = player_speed/2;
					if(!player.special_attack)
					{player.ry = -45;}
					break;
			
			case 2: 
					player.speed_z = player_speed/2; //up right
					player.speed_x = player_speed/2;
					player.ry = -135;
					break;
			case 3: 
					player.speed_z = player_speed/2;//up left
					player.speed_x = -player_speed/2;
					player.ry = 135;
					break;
			case 4: 
					player.speed_z = -player_speed/2;//down left
					player.speed_x = -player_speed/2;
					player.ry = 45;
					break;
			
			
			}
		}else if(player.dpad == 6)
		{
			switch(cam_number)
			{
			case 1: 
					player.speed_z = -player_speed/2;//down left
					player.speed_x = -player_speed/2;
					if(!player.special_attack)
					{player.ry = 45;}
					break;
			case 2: 
					player.speed_z = -player_speed/2;//down right
					player.speed_x = player_speed/2;
					player.ry = -45;
					break;
			
			case 3: 
					player.speed_z = player_speed/2; //up right
					player.speed_x = player_speed/2;
					player.ry = -135;
					break;
			case 4: 
					player.speed_z = player_speed/2;//up left
					player.speed_x = -player_speed/2;
					player.ry = 135;
					break;
			
			
			
			}
		}else if(player.dpad == 1)
		{
			switch(cam_number)
			{
			case 1: 
					player.speed_z = player_speed;//up
					player.speed_x = 0;
					if(!player.special_attack)
					{player.ry = 180;}
					break;
			case 2: 
					player.speed_x = -player_speed;//left
					player.ry = 90;
					player.speed_z = 0;
					break;
			case 3: 
					player.speed_z = -player_speed;//down
					player.speed_x = 0;
					JO_ZERO(player.ry);
					break;
			case 4: 
					player.speed_x = player_speed;//right
					player.speed_z = 0;
					player.ry = -90;
					break;
			}
		}
		
		else if (player.dpad == 5)
		{
		switch(cam_number)
			{
			case 1: 
					player.speed_z = -player_speed;//down
					player.speed_x = 0;
					if(!player.special_attack)
					{JO_ZERO(player.ry);}
					break;
			case 2: 
					player.speed_x = player_speed;//right
					player.speed_z = 0;
					player.ry = -90;
					break;
			case 3: 
					player.speed_z = player_speed;//up
					player.speed_x = 0;
					player.ry = 180;
					break;
			case 4: 
					player.speed_x = -player_speed;//left
					player.speed_z = 0;
					player.ry = 90;
					break;
			}
		}else if (player.dpad == 7)
		{
			switch(cam_number)
			{
			case 1: 
					player.speed_x = -player_speed;//left
					player.speed_z = 0;
					if(!player.special_attack)
					{player.ry = 90;}
					break;
			case 2: 
					player.speed_z = -player_speed;//down
					player.speed_x = 0;
					JO_ZERO(player.ry);
					break;
			case 3: 
					player.speed_x = player_speed;//right
					player.speed_z = 0;
					player.ry = -90;
					break;
			case 4: 	
					player.speed_z = player_speed;//up
					player.speed_x = 0;
					player.ry = 180;
					break;
			}
		}
		else if (player.dpad == 3)
		{
			switch(cam_number)
			{
			case 1: 
					player.speed_x = player_speed;//right
					player.speed_z = 0;
					if(!player.special_attack)
					{player.ry = -90;}
					break;
			case 2: 
					player.speed_z = player_speed;//up
					player.speed_x = 0;
					player.ry = 180;
					break;
			case 3: 
					player.speed_x = -player_speed;//left
					player.speed_z = 0;
					player.ry = 90;
					break;
			case 4: 
					player.speed_z = -player_speed;//down
					player.speed_x = 0;
					JO_ZERO(player.ry);
					break;
			}
		}
		else
		{
			player.speed_x = 0;
			player.speed_z = 0;
		}
	
	
		if (player.can_jump && (KEY_DOWN(0,PER_DGT_TA)) && player.type != 2 &&player.type !=4)
		{player_jump();
		
		}
		
		if ((KEY_DOWN(0,PER_DGT_TA)) && player.type == 2)
		{player_flap();
		
		}
		
		if ((KEY_PRESS(0,PER_DGT_TA)) && player.type == 4 && player.in_water)
		{player.speed_y = -player_speed;
		
		}
		
		if (KEY_PRESS(0,PER_DGT_TC)&& !show_debug && player.type == 4 && player.in_water)
		 {
			 player.speed_y = player_speed;
		 }
		
		 if (KEY_DOWN(0,PER_DGT_TC)&& show_debug)
		 {
			 if(player.type<4)
			 {change_player_type(player.type + 1);
			 }else
			 {
			change_player_type(0);	 
			 }
		 }
		 
		  if (KEY_DOWN(0,PER_DGT_TX))
		 {	
			if(cam_zoom_num == 4)
			{
			cam_zoom_num = 0;
			}else
			{
			 cam_zoom_num ++;
			}
		 }
		 
		 if (KEY_DOWN(0,PER_DGT_TB))
		 {	
			player.special_attack = true;
			effect(1,player.x,player.y,player.z);
			stop_sounds();
			jo_audio_play_sound_on_channel(&flap_sound, 2);
			if(!player.can_jump && player.type == 0)
			{
				//player dash down
			player.speed_y = 16.0f * framerate;	
			player.speed_x = 0;
			player.speed_z = 0;
			}
		 }
		 
		 
	
	if (KEY_DOWN(0,PER_DGT_ST))
	 {
		 if(game.pressed_start == false)
			{
				jo_nbg2_printf(0, 28, "                                       ");
				jo_nbg2_printf(0, 29, "                ");
				game.game_state = GAMESTATE_PAUSED;
				game.pause_menu = 0;
				jo_nbg2_printf(9, 12, "*");
			}
			game.pressed_start = true;
		}
    else
		{
			game.pressed_start = false;
		}
   
   if (KEY_DOWN(0,PER_DGT_TZ))
   {
	   ztClearText();	
	   show_debug ^= true;
   }
	
	
	if (cam_angle_y > 180)
		cam_angle_y -=360;
	else if (cam_angle_y <= -180)
		cam_angle_y +=360;
	
	
   
	
}
void            load_background()
{
    jo_img      bg;

    bg.data = JO_NULL;
    jo_tga_loader(&bg, "BG", "SKY.TGA", JO_COLOR_Transparent);
    jo_set_background_sprite(&bg, 0, 0);
    jo_free_img(&bg);
}







void                            draw_3d_planes(void)
{
	
	
	if (game.game_state != GAMESTATE_GAMEPLAY && game.game_state != GAMESTATE_MAP_BUILDER && game.game_state != GAMESTATE_TITLE_SCREEN)//&& game.game_state != GAMESTATE_TITLE_SCREEN
       return;
	int floor_y = - player.ysize;
	int floor_x = JO_DIV_BY_8(player.x);
	if(player.y <=0)
	{
	floor_y = JO_DIV_BY_8(player.y) - player.ysize;
	}
	int floor_z = JO_DIV_BY_8(player.z);
	
	//jo_enable_screen_transparency(NBG2ON,90);
	
	
	
   // SKY
    jo_3d_push_matrix();
	{
		//jo_3d_rotate_matrix(cam_angle_x, 0, cam_angle_z);
		jo_3d_rotate_matrix_rad(rot.rx, rot.ry, rot.rz);
		
		jo_3d_translate_matrixf(pos.x, pos.y, pos.z);
        jo_background_3d_plane_a_draw(true);
	}
	jo_3d_pop_matrix();
	pos.y += 0.1f;
	pos.x += 0.1f;
	//FLOOR
	 jo_3d_push_matrix();
	{
		//jo_3d_rotate_matrix(cam_angle_x, 0, cam_angle_z);
		jo_3d_rotate_matrix_rad(rot.rx, rot.ry, rot.rz);
		
		
		jo_3d_translate_matrixf(floor_x, floor_z, floor_y);
        jo_background_3d_plane_b_draw(true);
	}
	jo_3d_pop_matrix();

}

jo_palette          *my_tga_palette_handling(void)
{
    // We create a new palette for each image. It's not optimal but OK for a demo.
    jo_create_palette(&image_pal);
    return (&image_pal);
}



/*void				init_nbg1_image(void)
{
		jo_img_8bits    img2;
		img2.data = NULL;
		jo_tga_8bits_loader(&img2, "BG", "WATE3.TGA", 0);
		jo_vdp2_set_nbg1_8bits_image(&img2, image_pal.id, false);
		jo_free_img(&img2);
		
		jo_enable_screen_transparency(NBG1ON,80);
		
}*/

void                init_3d_planes(void)
{
    jo_img_8bits    img;

    ///Added by XL2 : turns off the TV while we load data
    slTVOff();    

    jo_enable_background_3d_plane(JO_COLOR_Black);

	// SKY
    img.data = JO_NULL;
	switch(game.level)
	{
		case 0: jo_tga_8bits_loader(&img, "BG", "SKY8.TGA", 0);
				break;
				
		case 1: jo_tga_8bits_loader(&img, "BG", "SKY8.TGA", 0);
				break;
				
		case 2: jo_tga_8bits_loader(&img, "BG", "SKY6.TGA", 0);
				break;
				
		case 3: jo_tga_8bits_loader(&img, "BG", "SKY7.TGA", 0);
		break;
	}
    
    jo_background_3d_plane_a_img(&img, image_pal.id, true, true);
    jo_free_img(&img);

    //FLOOR
    img.data = JO_NULL;
    switch(game.level)
	{
		case 0: jo_tga_8bits_loader(&img, "BG", "WATER.TGA", 0);
				break;
				
		case 1: jo_tga_8bits_loader(&img, "BG", "WATER.TGA", 0);
				break;
				
		case 2: jo_tga_8bits_loader(&img, "BG", "SAND.TGA", 0);
				break;
				
		case 3: jo_tga_8bits_loader(&img, "BG", "WATE3.TGA", 0);
		break;
	}
    jo_background_3d_plane_b_img(&img, image_pal.id, true, false);
    jo_free_img(&img);

   ///Added by XL2 : turns on the TV
	slTVOn();
    
}

/*void LoadLineColorTable() {
    int line;

    // Sets palette
    Uint16 * Line_Color_Pal0 = (Uint16 *)COLOR_RAM_ADR;
    for(line = 0; line < JO_TV_HEIGHT; line++)
    {
        int color = (line - 128) * 2;
        color = JO_ABS(color);

    	Line_Color_Pal0[line+32] = JO_COLOR_RGB(color,color,color);
    }

    // Set indexes to palette
    Line_Color_Pal0	=(Uint16 *)LINE_COLOR_TABLE;
    for(line = 0; line < JO_TV_HEIGHT; line++)
    {
    	Line_Color_Pal0[line] = line + ((256*3)+32);
    }

    slLineColDisp(RBG0ON);
    slLineColTable((void *)LINE_COLOR_TABLE);
    slColorCalc(CC_RATE | CC_2ND | RBG0ON);
    slColorCalcOn(RBG0ON);
    slColRateLNCL(0x0a);
}
*/
void            load_hud(void)
{
	int hud_sprite_id;
		
	hud_sprite_id = jo_sprite_add_tga_tileset("TEX", "HUD.TGA",JO_COLOR_Red,HUD_Tileset,3);
    player.health_sprite_id_1 = hud_sprite_id;
    player.gems_sprite_id_1 = hud_sprite_id + 1;
	player.effect_sprite_id_1 = hud_sprite_id + 2;
    

}

void	set_palette(Uint16 * palette, Uint16 TextureId)
{
    //For SGL only use slDMACopy instead : slDMACopy( (void*)palette, (void*)adr, sizeof(palette) );
    // slDMACopy( (void*)palette, (void*)returnLUTaddr(TextureId), sizeof(palette) );
	jo_dma_copy(palette, (void*)(returnLUTaddr(TextureId)), sizeof(Uint16)*16);

    //Only if you want to use the VDP2 CRAM for your sprite, else skip this
    //jo_palette_to_cram(palette, (void*)(returnCRAMaddr(TextureId)), 16);
}

/*void			    load_4bits_map_textures(void)
{
    jo_img_8bits    img4;
    jo_texture_definition   *ftexture;
	int id;
	
	for(int tex_num = 0; tex_num < 33; tex_num++)
	{	
	img_4bpp *current_texture=(img_4bpp *)map_img_data[tex_num];
	
	img4.data = NULL;
    img4.data = current_texture->palette_id;
    img4.width = JO_DIV_BY_2(current_texture->width);  //Since the image is 4 bits per pixel, we divide the width by 2 to fit each 2 pixels in 1 Byte
    img4.height = current_texture->height;
    id = jo_sprite_add_8bits_image(&img4);  //Adds the image in memory
    ftexture=&__jo_sprite_def[id];
    ftexture->width=current_texture->width;  //Ghetto technique for compatibility with Jo Engine, but trying to replace the sprite will throw an error
    //__jo_sprite_pic[id].color_mode=COL_16; // JJ won't compile with jo engine so removed
    ftexture->size = JO_MULT_BY_32(current_texture->width & 0x1f8) | ftexture->height;

    set_palette(current_texture->palette, id);
	}
}

void			    load_4bits_player_textures(void)
{
    jo_img_8bits    img4;
    jo_texture_definition   *ftexture;
	int id;
	
	for(int tex_num = 0; tex_num < 21; tex_num++)
	{	
	img_4bpp *current_texture=(img_4bpp *)player_img_data[tex_num];
	
	img4.data = NULL;
    img4.data = current_texture->palette_id;
    img4.width = JO_DIV_BY_2(current_texture->width);  //Since the image is 4 bits per pixel, we divide the width by 2 to fit each 2 pixels in 1 Byte
    img4.height = current_texture->height;
    id = jo_sprite_add_8bits_image(&img4);  //Adds the image in memory
    ftexture=&__jo_sprite_def[id];
    ftexture->width=current_texture->width;  //Ghetto technique for compatibility with Jo Engine, but trying to replace the sprite will throw an error
    //__jo_sprite_pic[id].color_mode=COL_16; // JJ won't compile with jo engine so removed
    ftexture->size = JO_MULT_BY_32(current_texture->width & 0x1f8) | ftexture->height;

    set_palette(current_texture->palette, id);
	}
}*/

void			load_binary(char * filename, void * startAddress)
{
char * stream;
 void * currentAddress;
unsigned int total_point =0;
unsigned int total_polygon=0;
unsigned int total_attribute=0;
unsigned int total_normal=0;
unsigned int total_collcubes=0;
int nxt = 0;

int line=2;
int length =0;
unsigned int att_tex = 0;
unsigned int att_plane = 0;
unsigned int att_flip = 0;
unsigned int g_count = 0;
unsigned int enemytype =0;
int enemyx =0;
int enemyy =0;
int enemyz =0;
int enemyxdist =0;
int enemyzdist =0;
unsigned int enemyspeed =0;
unsigned int enemyhits =0;
unsigned int puptype =0;
int pupx =0;
int pupy =0;
int pupz =0;
unsigned int current_model = 0;
int section_type = 0;
int section_x = 0;
int section_y = 0;
int section_z = 0;
unsigned int att_meshOn = 0;


stream = jo_fs_read_file(filename, &length);
currentAddress = startAddress;

jo_nbg2_printf(0, line, "LOADING....:                    ");
///total models
model_total = jo_swap_endian_uint(*((unsigned int *)(stream)));
//jo_nbg2_printf(0, line, "MODEL_TOTAL:         %d     ", model_total);
//line++;
nxt +=4;

///main loop
for (unsigned int s = 0; s< model_total; s++)
	{
		
		///current_model
		xpdata_[s]= currentAddress;
		current_model = jo_swap_endian_uint(*((unsigned int *)(stream+nxt)));
		//jo_nbg2_printf(0, line, "CURRENT_MODEL:         %d     ", model_total);
		//line++;
		nxt +=4;

		///points
		total_point = jo_swap_endian_uint(*((unsigned int *)(stream+nxt)));
		//jo_nbg2_printf(0, line, "LOADING....:                    ");
		nxt +=4;

		xpdata_[s]->pntbl = (POINT*)(xpdata_[s] + sizeof(unsigned int));

		for (unsigned int i = 0; i<total_point; i++)
		   {
			
			xpdata_[s]->pntbl[i][0] = jo_swap_endian_uint(*((unsigned int *)(stream+nxt)));
			nxt +=4;
			xpdata_[s]->pntbl[i][1] = jo_swap_endian_uint(*((unsigned int *)(stream+nxt)));
			nxt +=4;
			xpdata_[s]->pntbl[i][2] = jo_swap_endian_uint(*((unsigned int *)(stream+nxt)));
			nxt +=4;
			
			//jo_printf(0, line, "point: {%d,%d,%d} %d",point1,point2,point3, i);
			//line++;
			
			   

			}
			xpdata_[s]->nbPoint = total_point;
			//jo_nbg2_printf(0, line, "TOTAL_POINT:         %d     ", total_point);
			//line++;

		///polygon

		total_polygon = jo_swap_endian_uint(*((unsigned int *)(stream+nxt)));
		xpdata_[s]->pltbl = (POLYGON*)(xpdata_[s]->pntbl + sizeof(POINT)*total_point);
		//jo_nbg2_printf(0, line, "LOADING....:                    ");
		nxt +=4;

			for (unsigned int j = 0; j<total_polygon; j++)
		   {
			xpdata_[s]->pltbl[j].norm[0] = jo_swap_endian_uint(*((unsigned int *)(stream+nxt)));
			nxt+=4;
			xpdata_[s]->pltbl[j].norm[1] = jo_swap_endian_uint(*((unsigned int *)(stream+nxt)));
			nxt+=4;
			xpdata_[s]->pltbl[j].norm[2] = jo_swap_endian_uint(*((unsigned int *)(stream+nxt)));
			nxt+=4;
			xpdata_[s]->pltbl[j].Vertices[0] = jo_swap_endian_uint(*((unsigned int *)(stream+nxt)));
			nxt+=4;
			xpdata_[s]->pltbl[j].Vertices[1] = jo_swap_endian_uint(*((unsigned int *)(stream+nxt)));
			nxt+=4;
			xpdata_[s]->pltbl[j].Vertices[2] = jo_swap_endian_uint(*((unsigned int *)(stream+nxt)));
			nxt+=4;
			xpdata_[s]->pltbl[j].Vertices[3] = jo_swap_endian_uint(*((unsigned int *)(stream+nxt)));
			nxt+=4;
			
			
			}
			xpdata_[s]->nbPolygon = total_polygon;
			//jo_nbg2_printf(0, line, "TOTAL_POLYGON:       %d     ", total_polygon);
			//line++;


		///attribute

		total_attribute = jo_swap_endian_uint(*((unsigned int *)(stream+nxt)));
		xpdata_[s]->attbl = (ATTR*)(xpdata_[s]->pltbl + sizeof(POINT)*total_polygon);
	//	jo_nbg2_printf(0, line, "LOADING....:                    ");
		nxt +=4;

			for (unsigned int k = 0; k<total_attribute; k++)
		   {
			 
			 att_tex = jo_swap_endian_uint(*((unsigned int *)(stream+nxt)));
			nxt+=4;
			att_plane = jo_swap_endian_uint(*((unsigned int *)(stream+nxt)));
			nxt+=4;
			att_meshOn = jo_swap_endian_uint(*((unsigned int *)(stream+nxt)));
			nxt+=4;
			att_flip = jo_swap_endian_uint(*((unsigned int *)(stream+nxt)));
			nxt+=4;


			if(att_meshOn == 1)
			{
				if(att_plane == 1 && att_flip == 1)
				{
				ATTR bufAttr = ATTRIBUTE(Single_Plane, SORT_MAX, MAP_TILESET+att_tex, LUTidx(MAP_TILESET+att_tex), GRreal(g_count),Window_In|MESHon|HSSon|ECdis|CL32KRGB|CL_Gouraud, sprVflip, UseGouraud|UseClip);
				xpdata_[s]->attbl[k] = bufAttr;
				}else if(att_plane == 1)
				{
				ATTR bufAttr = ATTRIBUTE(Single_Plane, SORT_MAX, MAP_TILESET+att_tex, LUTidx(MAP_TILESET+att_tex), GRreal(g_count),Window_In|MESHon|HSSon|ECdis|CL32KRGB|CL_Gouraud, sprNoflip, UseGouraud|UseClip);
				xpdata_[s]->attbl[k] = bufAttr;		
				}else if(att_flip == 1)
				{
				ATTR bufAttr = ATTRIBUTE(Dual_Plane, SORT_MAX, MAP_TILESET+att_tex, LUTidx(MAP_TILESET+att_tex), GRreal(g_count),Window_In|MESHon|HSSon|ECdis|CL32KRGB|CL_Gouraud, sprVflip, UseGouraud|UseClip);
				xpdata_[s]->attbl[k] = bufAttr;		
				}else
				{
				ATTR bufAttr = ATTRIBUTE(Dual_Plane, SORT_MAX, MAP_TILESET+att_tex, LUTidx(MAP_TILESET+att_tex), GRreal(g_count),Window_In|MESHon|HSSon|ECdis|CL32KRGB|CL_Gouraud, sprNoflip, UseGouraud|UseClip);
				xpdata_[s]->attbl[k] = bufAttr;		
				}
				
			}else
			{
				
				if(att_plane == 1 && att_flip == 1)
				{
				ATTR bufAttr = ATTRIBUTE(Single_Plane, SORT_MAX, MAP_TILESET+att_tex, LUTidx(MAP_TILESET+att_tex), GRreal(g_count),Window_In|MESHoff|HSSon|ECdis|CL32KRGB|CL_Gouraud, sprVflip, UseGouraud|UseClip);
				xpdata_[s]->attbl[k] = bufAttr;
				}else if(att_plane == 1)
				{
				ATTR bufAttr = ATTRIBUTE(Single_Plane, SORT_MAX, MAP_TILESET+att_tex, LUTidx(MAP_TILESET+att_tex), GRreal(g_count),Window_In|MESHoff|HSSon|ECdis|CL32KRGB|CL_Gouraud, sprNoflip, UseGouraud|UseClip);
				xpdata_[s]->attbl[k] = bufAttr;		
				}else if(att_flip == 1)
				{
				ATTR bufAttr = ATTRIBUTE(Dual_Plane, SORT_MAX, MAP_TILESET+att_tex, LUTidx(MAP_TILESET+att_tex), GRreal(g_count),Window_In|MESHoff|HSSon|ECdis|CL32KRGB|CL_Gouraud, sprVflip, UseGouraud|UseClip);
				xpdata_[s]->attbl[k] = bufAttr;		
				}else
				{
				ATTR bufAttr = ATTRIBUTE(Dual_Plane, SORT_MAX, MAP_TILESET+att_tex, LUTidx(MAP_TILESET+att_tex), GRreal(g_count),Window_In|MESHoff|HSSon|ECdis|CL32KRGB|CL_Gouraud, sprNoflip, UseGouraud|UseClip);
				xpdata_[s]->attbl[k] = bufAttr;		
				}
				
				
			}
			g_count++;

			}
			
			//jo_nbg2_printf(0, line, "TOTAL_ATTRIBUTE:       %d     ", total_attribute);
			//line++;
			
		///normal
		total_normal = jo_swap_endian_uint(*((unsigned int *)(stream+nxt)));
		xpdata_[s]->vntbl = (VECTOR*)(xpdata_[s]->attbl + sizeof(POINT)*total_attribute);
		//jo_nbg2_printf(0, line, "LOADING....:                    ");
		nxt +=4;

			for (unsigned int l = 0; l<total_normal; l++)
		   {
			 
			xpdata_[s]->vntbl[l][0] = jo_swap_endian_uint(*((unsigned int *)(stream+nxt)));
			nxt+=4;
			xpdata_[s]->vntbl[l][1] = jo_swap_endian_uint(*((unsigned int *)(stream+nxt)));
			nxt+=4;
			xpdata_[s]->vntbl[l][2] = jo_swap_endian_uint(*((unsigned int *)(stream+nxt)));
			nxt+=4;
			

			

			}
			//jo_nbg2_printf(0, line, "TOTAL_NORMAL:       %d     ", total_normal);
			//line++;
			
			
		//********************************LP FAR MODEL************************************
		
		currentAddress = (void*) (xpdata_[s]->vntbl + sizeof(VECTOR)*total_normal);
		pdata_LP_[s]= currentAddress;

		///lp point
		total_point = jo_swap_endian_uint(*((unsigned int *)(stream+nxt)));
		//jo_nbg2_printf(0, line, "LOADING....:                    ");
		nxt +=4;

		pdata_LP_[s]->pntbl = (POINT*)(pdata_LP_[s] + sizeof(unsigned int));

		
		for (unsigned int i = 0; i<total_point; i++)
		{		
				
										
				pdata_LP_[s]->pntbl[i][0] = jo_swap_endian_uint(*((unsigned int *)(stream+nxt)));
				nxt+=4;
				pdata_LP_[s]->pntbl[i][1] = jo_swap_endian_uint(*((unsigned int *)(stream+nxt)));
				nxt+=4;
				pdata_LP_[s]->pntbl[i][2] = jo_swap_endian_uint(*((unsigned int *)(stream+nxt)));
				nxt+=4;
				
				
		}

		pdata_LP_[s]->nbPoint = total_point;
		
		//jo_nbg2_printf(0, line, "TOTAL_POINT_LP:     %d     ", total_point);
		//line++;

		///lp polygon
		total_polygon = jo_swap_endian_uint(*((unsigned int *)(stream+nxt)));
		//jo_nbg2_printf(0, line, "LOADING....:                    ");
		nxt +=4;

		pdata_LP_[s]->pltbl = (POLYGON*)(pdata_LP_[s]->pntbl + sizeof(POINT)*total_point);


		for (unsigned int j = 0; j<total_polygon; j++)
		{	
					
				pdata_LP_[s]->pltbl[j].norm[0] = jo_swap_endian_uint(*((unsigned int *)(stream+nxt)));
				nxt+=4;
				pdata_LP_[s]->pltbl[j].norm[1] = jo_swap_endian_uint(*((unsigned int *)(stream+nxt)));
				nxt+=4;
				pdata_LP_[s]->pltbl[j].norm[2] = jo_swap_endian_uint(*((unsigned int *)(stream+nxt)));
				nxt+=4;
				pdata_LP_[s]->pltbl[j].Vertices[0] = jo_swap_endian_uint(*((unsigned int *)(stream+nxt)));
				nxt+=4;
				pdata_LP_[s]->pltbl[j].Vertices[1] = jo_swap_endian_uint(*((unsigned int *)(stream+nxt)));
				nxt+=4;
				pdata_LP_[s]->pltbl[j].Vertices[2] = jo_swap_endian_uint(*((unsigned int *)(stream+nxt)));
				nxt+=4;
				pdata_LP_[s]->pltbl[j].Vertices[3] = jo_swap_endian_uint(*((unsigned int *)(stream+nxt)));
				nxt+=4;
		}
		//jo_nbg2_printf(0, line, "TOTAL_POLY_LP:      %d     ", total_polygon);
		//line++;


		pdata_LP_[s]->nbPolygon = total_polygon;

		///lp attribute
		total_attribute = jo_swap_endian_uint(*((unsigned int *)(stream+nxt)));
		//jo_nbg2_printf(0, line, "LOADING....:                    ");
		nxt +=4;

		pdata_LP_[s]->attbl = (ATTR*)(pdata_LP_[s]->pltbl + sizeof(POINT)*total_polygon);
		


			for (unsigned int k = 0; k<total_attribute; k++)
			{
				
				att_tex = jo_swap_endian_uint(*((unsigned int *)(stream+nxt)));
				nxt+=4;
				att_plane = jo_swap_endian_uint(*((unsigned int *)(stream+nxt)));
				nxt+=4;
				att_meshOn = jo_swap_endian_uint(*((unsigned int *)(stream+nxt)));
				nxt+=4;
				att_flip = jo_swap_endian_uint(*((unsigned int *)(stream+nxt)));
				nxt+=4;
						
				
				if(att_meshOn == 1)
				{
					if(att_plane == 1 && att_flip == 1)
					{
					ATTR bufAttr = ATTRIBUTE(Single_Plane, SORT_MAX, MAP_TILESET+att_tex, LUTidx(MAP_TILESET+att_tex), GRreal(0),Window_In|MESHon|HSSon|ECdis|SPdis|CL32KRGB|CL_Gouraud, sprNoflip, UseDepth|UseClip);
					pdata_LP_[s]->attbl[k] = bufAttr;
					}else if(att_plane == 1)
					{
					ATTR bufAttr = ATTRIBUTE(Single_Plane, SORT_MAX, MAP_TILESET+att_tex, LUTidx(MAP_TILESET+att_tex), GRreal(0),Window_In|MESHon|HSSon|ECdis|SPdis|CL32KRGB|CL_Gouraud, sprNoflip, UseDepth|UseClip);
					pdata_LP_[s]->attbl[k] = bufAttr;		
					}else if(att_flip == 1)
					{
					ATTR bufAttr = ATTRIBUTE(Dual_Plane, SORT_MAX, MAP_TILESET+att_tex, LUTidx(MAP_TILESET+att_tex), GRreal(0),Window_In|MESHon|HSSon|ECdis|SPdis|CL32KRGB|CL_Gouraud, sprNoflip, UseDepth|UseClip);
					pdata_LP_[s]->attbl[k] = bufAttr;		
					}else
					{
					ATTR bufAttr = ATTRIBUTE(Dual_Plane, SORT_MAX, MAP_TILESET+att_tex, LUTidx(MAP_TILESET+att_tex), GRreal(0),Window_In|MESHon|HSSon|ECdis|SPdis|CL32KRGB|CL_Gouraud, sprNoflip, UseDepth|UseClip);
					pdata_LP_[s]->attbl[k] = bufAttr;		
					}
					
				}else
				{					
				
					if(att_plane == 1 && att_flip == 1)
					{
					ATTR bufAttr = ATTRIBUTE(Single_Plane, SORT_MAX, MAP_TILESET+att_tex, LUTidx(MAP_TILESET+att_tex), GRreal(0),Window_In|MESHoff|HSSon|ECdis|SPdis|CL32KRGB|CL_Gouraud, sprNoflip, UseDepth|UseClip);
					pdata_LP_[s]->attbl[k] = bufAttr;
					}else if(att_plane == 1)
					{
					ATTR bufAttr = ATTRIBUTE(Single_Plane, SORT_MAX, MAP_TILESET+att_tex, LUTidx(MAP_TILESET+att_tex), GRreal(0),Window_In|MESHoff|HSSon|ECdis|SPdis|CL32KRGB|CL_Gouraud, sprNoflip, UseDepth|UseClip);
					pdata_LP_[s]->attbl[k] = bufAttr;		
					}else if(att_flip == 1)
					{
					ATTR bufAttr = ATTRIBUTE(Dual_Plane, SORT_MAX, MAP_TILESET+att_tex, LUTidx(MAP_TILESET+att_tex), GRreal(0),Window_In|MESHoff|HSSon|ECdis|SPdis|CL32KRGB|CL_Gouraud, sprNoflip, UseDepth|UseClip);
					pdata_LP_[s]->attbl[k] = bufAttr;		
					}else
					{
					ATTR bufAttr = ATTRIBUTE(Dual_Plane, SORT_MAX, MAP_TILESET+att_tex, LUTidx(MAP_TILESET+att_tex), GRreal(0),Window_In|MESHoff|HSSon|ECdis|SPdis|CL32KRGB|CL_Gouraud, sprNoflip, UseDepth|UseClip);
					pdata_LP_[s]->attbl[k] = bufAttr;		
					}
				}
				
				
			  
			}
		//jo_nbg2_printf(0, line, "TOTAL_ATTR_LP:      %d     ", total_attribute);
		//line++;
		
		//********************************************************************************

		///collpoints
		total_collcubes = jo_swap_endian_uint(*((unsigned int *)(stream+nxt)));

		currentAddress = (void*) (pdata_LP_[s]->attbl + sizeof(POINT)*total_attribute);
		cdata_[s]= currentAddress;
		cdata_[s]->cotbl = (COLLISON*)(cdata_[s] + sizeof(unsigned int));

		//jo_nbg2_printf(0, line, "LOADING....:                    ");
		nxt +=4;

			for (unsigned int m = 0; m<total_collcubes; m++)
		   {
			 
			cdata_[s]->cotbl[m].cen_x = jo_swap_endian_uint(*((unsigned int *)(stream+nxt)));
			nxt+=4;
			cdata_[s]->cotbl[m].cen_y = jo_swap_endian_uint(*((unsigned int *)(stream+nxt)));
			nxt+=4;
			cdata_[s]->cotbl[m].cen_z = jo_swap_endian_uint(*((unsigned int *)(stream+nxt)));
			nxt+=4;
			cdata_[s]->cotbl[m].x_size = jo_swap_endian_uint(*((unsigned int *)(stream+nxt)));
			nxt+=4;
			cdata_[s]->cotbl[m].y_size = jo_swap_endian_uint(*((unsigned int *)(stream+nxt)));
			nxt+=4;
			cdata_[s]->cotbl[m].z_size = jo_swap_endian_uint(*((unsigned int *)(stream+nxt)));
			nxt+=4;
			cdata_[s]->cotbl[m].att = jo_swap_endian_uint(*((unsigned int *)(stream+nxt)));
			nxt+=4;
			
			}
			cdata_[s]->nbCo = total_collcubes;
			//jo_nbg2_printf(0, line, "TOTAL_COLLCUBES:      %d     ", total_collcubes);
			//line++;
	
			currentAddress = (void*) (cdata_[s]->cotbl + sizeof(COLLISON)*total_collcubes);
		
			
	
	}
	
	
///create map layout

		total_sections = jo_swap_endian_uint(*((unsigned int *)(stream+nxt)));
		nxt+=4;
		
		for (unsigned int n = 0; n<total_sections; n++)
			{
		
			
		section_type = jo_swap_endian_uint(*((unsigned int *)(stream+nxt)));
		nxt+=4;
		section_x = jo_swap_endian_uint(*((unsigned int *)(stream+nxt)));
		nxt+=4;
		section_y = jo_swap_endian_uint(*((unsigned int *)(stream+nxt)));
		nxt+=4;
		section_z = jo_swap_endian_uint(*((unsigned int *)(stream+nxt)));
		nxt+=4;
		
		create_map_section(&map_section[n], section_type,section_x,section_y,section_z);	
		
			}
			
		//jo_nbg2_printf(0, line, "TOTAL_SECTIONS:         %d, g:%d     ", total_sections,g_count);
		//line++;


	
	
///ADD ENEMIES
	

//jo_nbg2_printf(0, line, "LOADING....:                    ");

enemy_total = jo_swap_endian_uint(*((unsigned int *)(stream+nxt)));
nxt +=4;

	for (int e = 0; e<enemy_total; e++)
		{
		
		enemytype = jo_swap_endian_uint(*((unsigned int *)(stream+nxt)));
		nxt+=4;
		enemyx = jo_swap_endian_uint(*((unsigned int *)(stream+nxt)));
		nxt+=4;
		enemyy = jo_swap_endian_uint(*((unsigned int *)(stream+nxt)));
		nxt+=4;
		enemyz = jo_swap_endian_uint(*((unsigned int *)(stream+nxt)));
		nxt+=4;
		enemyxdist = jo_swap_endian_uint(*((unsigned int *)(stream+nxt)));
		nxt+=4;
		enemyzdist = jo_swap_endian_uint(*((unsigned int *)(stream+nxt)));
		nxt+=4;
		enemyspeed = jo_swap_endian_uint(*((unsigned int *)(stream+nxt)));
		nxt+=4;
		enemyhits = jo_swap_endian_uint(*((unsigned int *)(stream+nxt)));
		nxt+=4;

		create_enemy(&enemies[e], enemytype, enemyx,enemyy, enemyz, enemyxdist,enemyzdist,enemyspeed,enemyhits);
		
		
		}
		
//jo_nbg2_printf(0, line, "ENEMY_TOTAL:         %d     ", enemy_total);
//line++;
	
//POWERUPS
//jo_nbg2_printf(0, line, "LOADING....:                    ");

powerup_total = jo_swap_endian_uint(*((unsigned int *)(stream+nxt)));
nxt +=4;

		for (int p = 0; p<powerup_total; p++)
		{
				
		puptype = jo_swap_endian_uint(*((unsigned int *)(stream+nxt)));
		nxt+=4;
		pupx = jo_swap_endian_uint(*((unsigned int *)(stream+nxt)));
		nxt+=4;
		pupy = jo_swap_endian_uint(*((unsigned int *)(stream+nxt)));
		nxt+=4;
		pupz = jo_swap_endian_uint(*((unsigned int *)(stream+nxt)));
		nxt+=4;
		create_powerup(&powerups[p],puptype,pupx, pupy, pupz);
		
		}
	//jo_nbg2_printf(0, line, "POWERUP_TOTAL:         %d     ", powerup_total);
//line++;

	line = 0;
	ztClearText();

	jo_free(stream);

}

void			load_textures(char * filename, int total_tiles)
{

	jo_sprite_free_from(game.map_sprite_id);
	game.map_sprite_id = jo_sprite_add_tga_tileset("TEX", filename,JO_COLOR_Red,MAP_Tileset,total_tiles);
	
			
	
}

void			load_level(void)
{

switch(game.level)
{
	case 1: //game.map_sprite_id = jo_sprite_add_tga_tileset("TEX", "L1.TGA",JO_COLOR_Red,MAP_Tileset,35);
			load_textures("L1.TGA",35);
			load_binary((char*)"L1.BIN", (void*)WORK_RAM_LOW);
			break;
			
	case 2: load_textures("L2.TGA",35);
			break;
			
	case 3: load_textures("L3.TGA",35);
	break;
	
}

	
}

void            extra_select(void)
{
	if (game.game_state != GAMESTATE_EXTRA_SELECT)
    return;
	
	
	jo_nbg2_printf(16, 8, "****EXTRA***");

    jo_nbg2_printf(10, 12, "OBJECT VIEWER");
	
    if (KEY_DOWN(0,PER_DGT_KL))
    {
        ztClearText();
		game.game_state = GAMESTATE_LEVEL_SELECT;
    }
       

   
    // did player one pause the game?
   if (KEY_DOWN(0,PER_DGT_ST))
    {
        if(game.pressed_start == false)
        {
			ztClearText();
			game.level = 1;
			load_level();
			game.game_state = GAMESTATE_OBJECT_VIEWER;	
        }
        game.pressed_start = true;
    }
    else
    {
        game.pressed_start = false;
    }

	
}

void            level_select(void)
{
	if (game.game_state != GAMESTATE_LEVEL_SELECT)
    return;
	
	
	jo_nbg2_printf(16, 8, "*LEVEL SELECT*");

    jo_nbg2_printf(10, 12, "LEVEL: %d",game.select_level);
	
    if (KEY_DOWN(0,PER_DGT_KU))
    {
        if (game.select_level == 1)
        game.select_level = LEVEL_MENU_MAX;
        else
        game.select_level --;
    }
       

    if (KEY_DOWN(0,PER_DGT_KD))
    {
        if (game.select_level == LEVEL_MENU_MAX)
        game.select_level = 1;
        else
        game.select_level ++;
    }
	
	
	if (KEY_DOWN(0,PER_DGT_KR))
    {
        ztClearText();
		game.game_state = GAMESTATE_EXTRA_SELECT;
    }

    // did player one pause the game?
   if (KEY_DOWN(0,PER_DGT_ST))
    {
        if(game.pressed_start == false)
        {
			game.level = game.select_level;
			load_level();
			//init_nbg1_image();
			//LoadLineColorTable();
            init_3d_planes();
			load_hud();
			//init_level();
			reset_demo();
			ztClearText();
			game.game_state = GAMESTATE_GAMEPLAY;	
        }
        game.pressed_start = true;
    }
    else
    {
        game.pressed_start = false;
    }

	
}

void gameLoop(void)
{
   	while (1)
    {
        slUnitMatrix(0);
		draw_3d_planes();
        my_gamepad();
		move_camera();
        my_draw();
		pause_game();
		map_builder();
		object_viewer();
		title_screen();
		level_select();
		extra_select();
		end_level();
		slSynch();
    }
}

void			load_player_and_enemies(void)
{
	
	
	//load_4bits_player_textures();
	jo_sprite_add_tga_tileset("TEX", "HAM.TGA",JO_COLOR_Red,HAM_Tileset,9);
	jo_sprite_add_tga_tileset("TEX", "ENEMY.TGA",JO_COLOR_Red,ENEMY_Tileset,14);
	
	create_player();
	
}



void			load_sound(void)
{
	jo_audio_load_pcm("STEP.PCM", JoSoundMono16Bit, &step_sound);
	jo_audio_load_pcm("DIE.PCM", JoSoundMono16Bit, &die_sound);
	die_sound.sample_rate = 27086;
	jo_audio_load_pcm("OUCH.PCM", JoSoundMono16Bit, &ouch_sound);
	//ouch_sound.sample_rate = 27086;
	jo_audio_load_pcm("HUH.PCM", JoSoundMono16Bit, &jump_sound);
	//jump_sound.sample_rate = 27086;
	jo_audio_load_pcm("LIGHT.PCM", JoSoundMono16Bit, &pup_sound);
	pup_sound.sample_rate = 27086;
	jo_audio_load_pcm("FLAP.PCM", JoSoundMono16Bit, &flap_sound);
	//flap_sound.sample_rate = 27086;
	jo_audio_load_pcm("BOING.PCM", JoSoundMono16Bit, &boing_sound);
	 boing_sound.sample_rate = 27086;
	 jo_audio_load_pcm("effect.PCM", JoSoundMono16Bit, &effect_sound);
	 effect_sound.sample_rate = 27086;
}

void                    load_nbg2_font(void)
{
    jo_img_8bits        img;

    img.data = NULL;
    jo_tga_8bits_loader(&img, JO_ROOT_DIR, "FONT.TGA", 2);
    jo_vdp2_set_nbg2_8bits_font(&img, " 0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ!\"?=%&',.()*+-/", image_pal.id, false, true);
    jo_free_img(&img);
}

/*void			init_gouraud_numbers(void)
{
	XPDATA * current_object;
	Sint16 object_num;
	
	Uint32 cnt, nbPt;
	
	for (object_num=0; object_num < 25; object_num++)
	{
	current_object=(XPDATA *)map_data[object_num];
	nbPt = current_object->nbPolygon;
	cnt = 0;
		
		for (cnt=0; cnt < nbPt; cnt++)
		{	
		current_object->attbl[cnt].gstb = GRreal(gouraud_counter);
		gouraud_counter++;
		}
	object_num++;
	}
	
}*/


void			jo_main(void)
{
	
    
	
	init_display();
	
	
	 /**Added by XL2 **/
	slDynamicFrame(ON); //Dynamic framerate, when the VDP1 can't fully draw to the framebuffer in the allocated amount of time (1/60, 2/60, etc.) it will continue drawing when it's ON. Else it will try to complete drawing by skipping lines and details and finish in the allocated time. If the app runs well with the value at OFF, leave it at OFF!
    SynchConst=(Sint8)2;  //Framerate control. 1/60 = 60 FPS, 2/60 = 30 FPS, etc.
	framerate=2;
	
	jo_set_tga_palette_handling(my_tga_palette_handling);
	
	load_nbg2_font();
	
	game.select_level = 1;
	load_player_and_enemies();
	load_sound();	
	
	game.map_sprite_id = jo_sprite_add_tga_tileset("TEX", "TITLE.TGA",JO_COLOR_Red,MAP_Tileset,35);
	load_binary((char*)"TITLEC.BIN", (void*)WORK_RAM_LOW);
	//game.level = 0;
	//reset_demo();
	//load_level();	
	//init_gouraud_numbers();
	init_3d_planes();
		
	pos.x = 800.0;
	pos.y = 800.0;
	pos.z = 35.0;

	rot.rx = JO_DEG_TO_RAD(90.0);
	rot.ry = JO_DEG_TO_RAD(0.0);
	rot.rz = JO_DEG_TO_RAD(45.0);
	
	game.game_state = GAMESTATE_TITLE_SCREEN;
	slZdspLevel(5);
	gameLoop();
}

/*
** END OF FILE
*/
