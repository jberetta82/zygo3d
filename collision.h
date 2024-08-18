#ifndef __Collision_H__
# define __Collision_H__

///COLLISION V4///


typedef struct {
	
    Sint16	 cen_x;	//x pos of coll cube		
	Sint16  cen_y;			
	Sint16  cen_z;	
	Sint16	 x_size; //x size of coll cube
	Sint16	 y_size;
	Sint16	 z_size;	
	Uint8  att;	//attribute		
	//int	 gru;	//group (model)	
} COLLISON;

typedef struct {
	COLLISON	*cotbl;		/* ポリゴン定義テーブル  collision definition table*/
	Uint32		 nbCo;		/* ポリゴンの数 number of coll cubes*/
}CDATA;







typedef struct section_data{
    //  Render flag
    //bool alive;
	
	//type
	Uint8 type;
	//bool door_open;
	//bool open_door;
	//bool close_door;
	//collisions
	
    // Position
    Sint16 x;
    Sint16 y;
    Sint16 z;
	
	//position adj (open doors)
	Sint16 tx;
	Sint16 ty;
	Sint16 tz;

    // Rotation
    //int rx;
    //int ry;
    //int rz;
	
	// Size (Hitboxes)
	//int xsize;
	//int	ysize;
	//int zsize;
	
	//model
	XPDATA	*map_model;
	PDATA	*map_model_lp;
	//collisions
	CDATA		*a_cdata;
	COLLISON	*a_collison;
	
	//centre of map adjustment
	//int map_adjust;
	//int map_yadjust;
	
	//mesh
	

   
   
}level_section;

level_section		map_section[];







#endif /* !__Collision_H__ */
