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

#ifndef __HAMSTER_H__
# define __HAMSTER_H__

# define WORLD_DEFAULT_X                (0)
# define WORLD_DEFAULT_Y                (-64)//-64
# define WORLD_DEFAULT_Z                (0)


# define CAM_DEFAULT_X                (0)
# define CAM_DEFAULT_Y                (-24) //-64
# define CAM_ZOOM_1		              (50) 
# define CAM_ZOOM_2                    (100) 
# define CAM_ZOOM_3                   (200)
# define CAM_DEFAULT_Z                (-150)//-150
# define CAM_DIST_DEFAULT				(150)
# define CAM_SPEED                		(10)//5
# define CAM_MAX_DIST              		(400)
# define COLL_DIST              		(512)
# define DRAW_DISTANCE					(448)//512
# define DRAW_DISTANCE_2					(704)//768
# define DRAW_DISTANCE_X				(300)
# define DRAW_DISTANCE_3					(300)
# define DRAW_DISTANCE_MAX					(1024)//768
//# define FOV						(70)

# define DOOR_SPEED					(2)
# define DOOR_RANGE					(8)

//define game states
#define GAMESTATE_UNINITIALIZED         (0)
#define GAMESTATE_TITLE_SCREEN          (1)
#define GAMESTATE_LEVEL_SELECT          (2)
#define GAMESTATE_GAMEPLAY              (3)
#define GAMESTATE_PAUSED                (4)
#define GAMESTATE_GAME_OVER             (5)
#define GAMESTATE_VICTORY               (6)
#define GAMESTATE_MAP_BUILDER           (7)
#define GAMESTATE_END_LEVEL				(8)
#define GAMESTATE_OBJECT_VIEWER			(9)
#define GAMESTATE_EXTRA_SELECT			(10)

//define menu options
#define PAUSE_MENU_MAX               (4)
#define LEVEL_MENU_MAX               (3)
#define END_LEVEL_MENU_MAX           (2)

#define ANIM_SPEED               (3)
#define MAX_PLAYERS         (2)
#define PLAYER_SPEED         (2)
#define PLAYER_HURT_TIMER        (60)
#define ENEMY_HURT_TIMER        (20)
#define ENEMY_JUMP_TIMER        (60)
#define PUP_TIMER       	 (380)
#define NO_RAMP_COLLISION		(0)
#define CEILING_COLLISION		(999999)

enum player_state {idle,walking,running,jumping,falling,on_ladder_x,on_ladder_y,on_ceiling, special_attack, slam_attack};

typedef struct  _PLAYER
{
	Sint16 health_sprite_id_1;
	Sint16 gems_sprite_id_1;
	Sint16 effect_sprite_id_1;
    bool alive;
	Uint8 state;
	Uint8 health;
	bool hurt;
	float speed;
	Uint8 type;
	Sint16 hud_sprite;
	bool can_be_hurt;
	Sint16 hurt_timer;
	Sint16 pup_timer;
	bool mutate;
	Sint8 gamepad;
	bool pressed_start;
	bool pressed_up;
	bool pressed_down;
	Sint8 dpad;
	Sint16 gems;
	bool on_ladder_x;	
	bool on_ladder_z;
	bool on_ceiling;
	bool in_water;
	bool flapping;
	   
	// Start Position
    Sint16 start_x;
    Sint16 start_y;
    Sint16 start_z;
	// Position
    Sint16 x;
    Sint16 y;
    Sint16 z;
	Sint16 nextx;
	Sint16 nexty;
	Sint16 nextz;
	float delta_x;
	float delta_y;
	float delta_z;
	

    // Rotation
    Sint16 rx;
    Sint16 ry;
    Sint16 rz;
	
	//head Rotation
	Sint16 head_rx;
	Sint16 head_ry;
	Sint16 head_rz;
	
	//body Rotation
	Sint16 body_rx;
	Sint16 body_ry;
	Sint16 body_rz;
	
	//arm Rotation
	Sint16 larm_rx;
	Sint16 larm_ry;
	Sint16 larm_rz;
	Sint16 rarm_rx;
	Sint16 rarm_ry;
	Sint16 rarm_rz;
	
	//leg Rotation
	Sint16 lleg_rx;
	Sint16 lleg_ry;
	Sint16 lleg_rz;
	Sint16 rleg_rx;
	Sint16 rleg_ry;
	Sint16 rleg_rz;
	
	//leg position
	Sint16 lleg_x;
	Sint16 lleg_y;
	Sint16 lleg_z;
	Sint16 rleg_x;
	Sint16 rleg_y;
	Sint16 rleg_z;
	
	//shadow
	float shadow_y;
	float shadow_speed;
	float shadow_size;
	Sint16 current_shadow_map_section;
	
	//strike
	Sint16 strike_timer;
	bool strike; 
	
	//kick
	Sint16 kick_timer;
	bool kick; 
	
	//special attack
	Sint16 special_attack_timer;
	bool special_attack; 
	float special_attack_size;
	
	Sint16 anim_speed;
	
	// Size (Hitboxes)
    Sint16 xsize;
    Sint16 ysize;
    Sint16 zsize;
	
	float speed_x;
	float speed_y;
	float speed_z;
	
	Uint16 current_map_section;
	Uint16 current_collision;
	Uint16 current_shadow_collision;
	bool can_jump;
	float jump_height;
	
	//ramp
	float R_int_height;
	float int_height;
	Sint16 ramp_height_adj;
	
	//dust clouds when walking
	float left_cloud_size;
	float right_cloud_size;
	Sint16 left_cloud_x;
	float left_cloud_y;
	Sint16 left_cloud_z;
	Sint16 right_cloud_x;
	Sint16 right_cloud_y;
	Sint16 right_cloud_z;
	
	//hud
	Sint16 hud_gem_ry;
	
	Uint16 r;
	Uint16 g;
	Uint16 b;
	
	Sint16 effect_x;
	Sint16 effect_y;
	Sint16 effect_z;
	float effect_size;
	Uint8 effect_type;


}               player_params;

extern player_params player;


typedef struct	_ENEMY
{
	bool alive;
	Sint16 health;
	Sint16 start_health;
	bool hurt;
	Uint8 type;
	Sint16 hud_sprite;
	bool can_be_hurt;
	Sint16 hurt_timer;
	Sint16 anim_speed;
	Sint16 death_timer;

    // Start Position
    Sint16 start_x;
    Sint16 start_y;
    Sint16 start_z;
		
	// Position
	Sint16 x;
    Sint16 y;
    Sint16 z;
	Sint16 max_speed;
	Sint16 speed_x;
	float speed_y;
	Sint16 speed_z;
	Sint16 xdist;
	Sint16 zdist;
	//bool flip_direction;
	Sint16 waypoint;
	
	// Size
    Sint16 xsize;
    Sint16 ysize;
    Sint16 zsize;
	
	//air bubbles
	float air_bubble_size;
	Sint16 air_bubble_x;
	float air_bubble_y;
	Sint16 air_bubble_z;
	

    // Rotation
    Sint16 rx;
    float ry;
    Sint16 rz;
	
	//head Rotation
	Sint16 head_rx;
	Sint16 head_ry;
	Sint16 head_rz;
	
	//body Rotation
	Sint16 body_rx;
	Sint16 body_ry;
	Sint16 body_rz;
	
	//arm Rotation
	Sint16 larm_rx;
	Sint16 larm_ry;
	Sint16 larm_rz;
	Sint16 rarm_rx;
	Sint16 rarm_ry;
	Sint16 rarm_rz;
	
	//leg Rotation
	Sint16 lleg_rx;
	Sint16 lleg_ry;
	Sint16 lleg_rz;
	Sint16 rleg_rx;
	Sint16 rleg_ry;
	Sint16 rleg_rz;
	
	//target
	Sint16 target;
	Sint16 tx;
	Sint16 ty;
	Sint16 tz;
	float tr;
	
	//melee attack
	Sint16 attack_timer;
	bool attack; 
	
	//jump
	Sint16 jump_timer;
	
	//die effect
	float effect_size;
	
	
	
	//projectile is active
	bool projectile_alive;
	Sint16 shoot_counter;
	Sint16 shoot_wait;
	bool can_shoot;
	
	//projectile position
	Sint16	px;
	Sint16 py;
	Sint16	pz;
	
	float speed_px;
	float speed_py;
	float speed_pz;
	
	//projectile target
	Sint16 ptx;
	Sint16 pty;
	Sint16 ptz;
	
	//projectile direction
	float pr;
	
	
	//ramp
	float R_int_height;
	float int_height;
	Sint16 ramp_height_adj;
	Sint16 current_map_section;
	bool can_jump;
	
	//death
	Sint16 explosion_size;
	
	

	
}				enemy;
extern enemy 	enemies[];

typedef struct	_POWERUP
{

	Uint8			type;
	Sint16			x;
	Sint16			y;
	Sint16			z;
	Sint16			ry;
	bool		used;
	bool		used_saved;
	Uint8			lev;
	//model
	XPDATA	*pup_model;
	

	
}				powerup;
extern powerup	powerups[];

typedef struct  _GAME
{
    // game state variables
	Sint16 		map_sprite_id;
    Uint8         game_state;
    Uint8         pause_menu;
    bool        pressed_start;
	bool        pressed_left;
	bool        pressed_right;
    bool        pressed_up;
    bool        pressed_down;
	bool        pressed_X;
	Uint8		players;
	Uint16		level_1_gems;
	Uint8		end_level_menu;
	Uint8 		level_select_menu;
	Uint8		select_level;
	Uint8		level;


}               game_params;
extern game_params game;



#endif /* !__HAMSTER_H__ */

/*
** END OF FILE
*/
