#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <time.h>

#include "main.h"
#include "SDL.h"
#include <SDL_ttf.h>



void apply_autoscale(settings *sim_set, planet objects[]){
int i;
double dx, dy, dz;
double r, r_max=0.;

// Reset rotation
sim_set->x_rot = 0.;
sim_set->y_rot = 0.;

center_at_cms(sim_set, objects);

// Find maximum cms distance
for(i=0; i<sim_set->n_bodies; i++){

	dx = objects[i].pos[0] - sim_set->cms[0];
	dy = objects[i].pos[1] - sim_set->cms[1];
	dz = objects[i].pos[2] - sim_set->cms[2];

	// calculate x-y distance
	r = sqrt( dx*dx + dy*dy + dz*dz );

	if ( r > r_max ) r_max = r;

}

sim_set->scale = 2.05*r_max;

center_at_cms(sim_set, objects);

// Reset scale min/max
sim_set->scale_max = sim_set->scale;
sim_set->scale_min = sim_set->scale;

}




void center_at_cms(settings *sim_set, planet objects[]){
int i, j;
double scale_factor;
double x,y,z;
double sin_y_rot, cos_y_rot, sin_x_rot, cos_x_rot;
double sx, sy;

sin_y_rot = sin(sim_set->y_rot*deg_to_rad);
cos_y_rot = cos(sim_set->y_rot*deg_to_rad);

sin_x_rot = sin(sim_set->x_rot*deg_to_rad);
cos_x_rot = cos(sim_set->x_rot*deg_to_rad);

// Clear CMS
for(j=0; j<3; j++) sim_set->cms[j] = 0.;

// Calculate CMS coordinates
for(i=0; i<sim_set->n_bodies; i++){

	for(j=0; j<3; j++) sim_set->cms[j] += objects[i].pos[j]*objects[i].mass;

}

for(j=0; j<3; j++) sim_set->cms[j] = sim_set->cms[j]/sim_set->m_tot;

x = sim_set->cms[0];
y = sim_set->cms[1];
z = sim_set->cms[2];

scale_factor = sim_set->res_x/sim_set->scale;

sx = scale_factor*x*cos_y_rot+scale_factor*z*sin_y_rot;
sy = scale_factor*x*sin_x_rot*sin_y_rot+scale_factor*y*cos_x_rot-scale_factor*z*sin_x_rot*cos_y_rot; 

sim_set->center_screen_x = 0.5*sim_set->res_x - sx;
sim_set->center_screen_y = 0.5*sim_set->res_y - sy;

}


void create_screenshot(SDL_Renderer *renderer, settings *sim_set){
char path[18];

sprintf(path, "screenshot_%d.bmp", sim_set->screenshot_counter); // puts string into buffer

SDL_Surface *sshot = SDL_CreateRGBSurface(0, sim_set->res_x, sim_set->res_y, 32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000);
SDL_RenderReadPixels(renderer, NULL, SDL_PIXELFORMAT_ARGB8888, sshot->pixels, sshot->pitch);
SDL_SaveBMP(sshot, path);
SDL_FreeSurface(sshot);

sim_set->screenshot_counter = sim_set->screenshot_counter + 1;
sim_set->screenshot_trigger = 0;

}



void create_auto_screenshot(SDL_Renderer *renderer, settings *sim_set){
char path[18];

sprintf(path, "auto_%d.bmp", sim_set->auto_screenshot_counter); // puts string into buffer

SDL_Surface *sshot = SDL_CreateRGBSurface(0, sim_set->res_x, sim_set->res_y, 32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000);
SDL_RenderReadPixels(renderer, NULL, SDL_PIXELFORMAT_ARGB8888, sshot->pixels, sshot->pitch);
SDL_SaveBMP(sshot, path);
SDL_FreeSurface(sshot);

sim_set->auto_screenshot_counter++;

}




void switch_interactive_mode(settings *sim_set){

if ( sim_set->interactive_mode == 0 ){
sim_set->interactive_mode = 1;
}
else{
sim_set->interactive_mode = 0;
}

}




void switch_pause_mode(settings *sim_set){

if ( sim_set->paused == 0 ){
sim_set->paused = 1;
}
else{
sim_set->paused = 0;
}

}




int processEvents(settings *sim_set, planet objects[]){
SDL_Event event;
int done=0;
int x, y;
int delta_x, delta_y;

while(SDL_PollEvent(&event)){

	switch(event.type){

		case SDL_WINDOWEVENT_CLOSE:
			{
				done = 1;
			}
			break;

		case SDL_KEYDOWN:
			{
			switch(event.key.keysym.sym){

				case SDLK_a:
					apply_autoscale(sim_set, objects);
				break;

				case SDLK_o:
					sim_set->center_screen_x = 0.5*sim_set->res_x;
					sim_set->center_screen_y = 0.5*sim_set->res_y;
				break;

				case SDLK_c:
					center_at_cms(sim_set, objects);
				break;

				case SDLK_r:
					sim_set->timestep_max = sim_set->timestep;
					sim_set->timestep_min = sim_set->timestep;
					sim_set->scale_min = sim_set->scale;
					sim_set->scale_max = sim_set->scale;
				break;

				case SDLK_DOWN:
					sim_set->x_rot = sim_set->x_rot-1.;
				break;

				case SDLK_UP:
					sim_set->x_rot = sim_set->x_rot+1.;
				break;

				case SDLK_RIGHT:
					sim_set->y_rot = sim_set->y_rot+1.;
				break;

				case SDLK_LEFT:
					sim_set->y_rot = sim_set->y_rot-1.;
				break;

				case SDLK_n:
					sim_set->x_rot = 0.;
					sim_set->y_rot = 0.;
				break;

				case SDLK_ESCAPE:
					done = 1;
				break;

				case SDLK_SPACE:
					switch_pause_mode(sim_set);
				break;

				case SDLK_i:
					switch_interactive_mode(sim_set);
				break;

				case SDLK_s:
					sim_set->screenshot_trigger = 1;
				break;

				case SDLK_PAGEUP:
					zoom_in(sim_set);						
				break;

				case SDLK_PAGEDOWN:
					zoom_out(sim_set);	
				break;	
			}
		}
		break;

		case SDL_MOUSEWHEEL:
			{
			
			if ( event.wheel.y > 0 ){
				zoom_in_at_mouse_position(sim_set);		
			}
			else{
				zoom_out_at_mouse_position(sim_set);
			}
			}
			break;
			

		case SDL_MOUSEBUTTONDOWN:
			{

			if( event.button.state == SDL_PRESSED && event.button.button == SDL_BUTTON_LEFT){

				// Get mouse position
				SDL_GetMouseState( &x, &y );

				// Save mouse position at mouse click start
				sim_set->start_x = x;
				sim_set->start_y = y;

			}
			if( event.button.state == SDL_PRESSED && event.button.button == SDL_BUTTON_RIGHT && sim_set->selected_object == -1){
			
				// Get mouse position
				SDL_GetMouseState( &x, &y );
/*
				// Check if a planet was clicked
				for (i=0;i<sim_set->n_bodies;i++){

					if( x >= objects[i].select_box_x[0] && x <= objects[i].select_box_x[1] && y >= objects[i].select_box_y[0] && y <= objects[i].select_box_y[1]){
						sim_set->selected_object = i;
						break;
					}
					else{
						// Reset selection
						sim_set->selected_object = -1;
					}

				}
*/

			}

			}
			break;


		case SDL_MOUSEBUTTONUP:
			{

			if( event.button.state == SDL_RELEASED && event.button.button == SDL_BUTTON_LEFT ){

				// Get mouse position
				SDL_GetMouseState( &x, &y );

				// Assign position to settings type
				sim_set->end_x = x;
				sim_set->end_y = y;

				// calculate motion delta
				delta_x = sim_set->start_x-sim_set->end_x;
				delta_y = sim_set->start_y-sim_set->end_y;			

				sim_set->center_screen_x = sim_set->center_screen_x - delta_x;
				sim_set->center_screen_y = sim_set->center_screen_y - delta_y;		

			}

			}
			break;


		case SDL_QUIT:
			done = 1;
			break;
	}

}

return done;

}





void Clear_to_black(SDL_Renderer *renderer, settings *sim_set){

// Set drawing color to black
SDL_SetRenderDrawColor(renderer, 0,0,0,255);				// (Red, Green, Blue, Alpha)

// Clear the screen to black
SDL_RenderClear(renderer);

}


void Draw_Background(SDL_Renderer *renderer, SDL_Texture *background, settings *sim_set){
SDL_Rect stretchRect;

if ( sim_set->draw_background == 1){
stretchRect.x = 0; 
stretchRect.y = 0; 
stretchRect.w = sim_set->res_x; 
stretchRect.h = sim_set->res_y; 

SDL_RenderCopy(renderer, background, NULL, &stretchRect);
}
else{
Clear_to_black(renderer, sim_set);
}

}




void Render_Screen(SDL_Renderer *renderer){
// Present now what we've drawn
SDL_RenderPresent(renderer);
}



void load_texture(SDL_Renderer *renderer, SDL_Texture **texture, char* path){
SDL_Surface* img_surf;

img_surf = SDL_LoadBMP(path);

if(img_surf==NULL){
printf("Error.");
SDL_Quit();
}

SDL_SetColorKey( img_surf, SDL_TRUE, SDL_MapRGB(img_surf->format,255, 0, 255) ); 
*texture = SDL_CreateTextureFromSurface(renderer, img_surf);

SDL_FreeSurface(img_surf);

}





void load_object_textures(SDL_Renderer *renderer, settings *sim_set){

load_texture(renderer, &sim_set->icon_sun, "sprites/sun_icon.bmp");
load_texture(renderer, &sim_set->icon_mercury, "sprites/mercury_icon.bmp");
load_texture(renderer, &sim_set->icon_venus, "sprites/venus_icon.bmp");
load_texture(renderer, &sim_set->icon_earth, "sprites/earth_icon.bmp");
load_texture(renderer, &sim_set->icon_mars, "sprites/mars_icon.bmp");
load_texture(renderer, &sim_set->icon_moon, "sprites/moon_icon.bmp");
load_texture(renderer, &sim_set->icon_mars, "sprites/mars_icon.bmp");
load_texture(renderer, &sim_set->icon_jupiter, "sprites/jupiter_icon.bmp");
load_texture(renderer, &sim_set->icon_saturn, "sprites/saturn_icon.bmp");
load_texture(renderer, &sim_set->icon_uranus, "sprites/uranus_icon.bmp");
load_texture(renderer, &sim_set->icon_neptune, "sprites/neptune_icon.bmp");
load_texture(renderer, &sim_set->icon_pluto, "sprites/pluto_icon.bmp");

}






void render_icon(SDL_Renderer *renderer, planet *object, settings *sim_set){
SDL_Rect stretchRect;

stretchRect.x = round(object->screen_pos[0] - 0.5*object->icon_size); 
stretchRect.y = round(object->screen_pos[1] - 0.5*object->icon_size); 
stretchRect.w = round(object->icon_size); 
stretchRect.h = round(object->icon_size); 

switch(object->icon_num) {
	case 0: SDL_RenderCopy(renderer, sim_set->icon_sun, NULL, &stretchRect) ; break;
	case 1: SDL_RenderCopy(renderer, sim_set->icon_mercury, NULL, &stretchRect) ; break;
	case 2: SDL_RenderCopy(renderer, sim_set->icon_venus, NULL, &stretchRect) ; break;
	case 3: SDL_RenderCopy(renderer, sim_set->icon_earth, NULL, &stretchRect) ; break;
	case 4: SDL_RenderCopy(renderer, sim_set->icon_mars, NULL, &stretchRect) ; break;
	case 5: SDL_RenderCopy(renderer, sim_set->icon_jupiter, NULL, &stretchRect) ; break;
	case 6: SDL_RenderCopy(renderer, sim_set->icon_saturn, NULL, &stretchRect) ; break;
	case 7: SDL_RenderCopy(renderer, sim_set->icon_uranus, NULL, &stretchRect) ; break;
	case 8: SDL_RenderCopy(renderer, sim_set->icon_neptune, NULL, &stretchRect) ; break;
	case 9: SDL_RenderCopy(renderer, sim_set->icon_pluto, NULL, &stretchRect) ; break;
	case 10: SDL_RenderCopy(renderer, sim_set->icon_moon, NULL, &stretchRect) ; break;
	default: SDL_RenderCopy(renderer, sim_set->icon_sun, NULL, &stretchRect) ; break;
}


}




void render_icon_size_brightness(SDL_Renderer *renderer, planet *object, settings *sim_set, double size, double brightness){
SDL_Rect stretchRect;
SDL_Texture *icon_modified;
int scale;

stretchRect.x = round(object->screen_pos[0] - 0.5*size); 
stretchRect.y = round(object->screen_pos[1] - 0.5*size); 
stretchRect.w = round(size); 
stretchRect.h = round(size); 

scale = (int)(255*brightness);

switch(object->icon_num) {
	case 0: icon_modified = sim_set->icon_sun; break;
	case 1: icon_modified = sim_set->icon_mercury; break;
	case 2: icon_modified = sim_set->icon_venus; break;
	case 3: icon_modified = sim_set->icon_earth; break;
	case 4: icon_modified = sim_set->icon_mars; break;
	case 5: icon_modified = sim_set->icon_jupiter; break;
	case 6: icon_modified = sim_set->icon_saturn; break;
	case 7: icon_modified = sim_set->icon_uranus; break;
	case 8: icon_modified = sim_set->icon_neptune; break;
	case 9: icon_modified = sim_set->icon_pluto; break;
	case 10: icon_modified = sim_set->icon_moon; break;
	default: icon_modified = sim_set->icon_sun; break;
}

SDL_SetTextureColorMod(icon_modified, scale, scale, scale);

SDL_RenderCopy(renderer, icon_modified, NULL, &stretchRect);

}




void render_all_bodies(SDL_Renderer *renderer, planet objects[], settings *sim_set){
int i;

for(i=0; i<sim_set->n_bodies; i=i+1){
	render_icon_size_brightness(renderer, &objects[i], sim_set, 10, 1.);
	//render_icon(renderer, &objects[i], sim_set);
}

}






int compare_structs(const void *a, const void *b){
     
planet *struct_a = (planet *) a;
planet *struct_b = (planet *) b;
     
if (struct_a->z_projected < struct_b->z_projected) return 1;
else if (struct_a->z_projected == struct_b->z_projected) return 0;
else return -1;
     
}




void render_all_bodies_2D(SDL_Renderer *renderer, planet objects[], settings *sim_set){
int i, i_plot;
double sin_y_rot, cos_y_rot, sin_x_rot, cos_x_rot;

// Calculate screen coordinates
get_planar_screen_coordinates(objects, sim_set);

sin_y_rot = sin(sim_set->y_rot*deg_to_rad);
cos_y_rot = cos(sim_set->y_rot*deg_to_rad);

sin_x_rot = sin(sim_set->x_rot*deg_to_rad);
cos_x_rot = cos(sim_set->x_rot*deg_to_rad);

// Get projected distances 
for(i=0; i<sim_set->n_bodies; i++){

	objects[i].z_projected = (-objects[i].pos[0]*cos_x_rot*sin_y_rot+objects[i].pos[1]*sin_x_rot+objects[i].pos[2]*cos_x_rot*cos_y_rot);
	objects[i].plot_order = i;

}

qsort(objects, sim_set->n_bodies, sizeof(objects[0]), compare_structs);

for(i=sim_set->n_bodies-1; i>=0; i--){

	i_plot = objects[i].plot_order;

	render_icon(renderer, &objects[i_plot], sim_set);
}

}





void render_all_bodies_3D(SDL_Renderer *renderer, planet objects[], settings *sim_set){
int i, i_plot;
double sin_y_rot, cos_y_rot, sin_x_rot, cos_x_rot;
double icon_size, min_size, delta;
double brightness;
double d_scale;

// Calculate screen coordinates
get_planar_screen_coordinates(objects, sim_set);

sin_y_rot = sin(sim_set->y_rot*deg_to_rad);
cos_y_rot = cos(sim_set->y_rot*deg_to_rad);

sin_x_rot = sin(sim_set->x_rot*deg_to_rad);
cos_x_rot = cos(sim_set->x_rot*deg_to_rad);

// Get projected distances 
for(i=0; i<sim_set->n_bodies; i++){

	objects[i].z_projected = (-objects[i].pos[0]*cos_x_rot*sin_y_rot+objects[i].pos[1]*sin_x_rot+objects[i].pos[2]*cos_x_rot*cos_y_rot);
	objects[i].plot_order = i;

}

qsort(objects, sim_set->n_bodies, sizeof(objects[0]), compare_structs);
delta = 0.5*sim_set->scale;

for(i=sim_set->n_bodies-1; i>=0; i--){

	i_plot = objects[i].plot_order;

	if( objects[i_plot].z_projected > delta ) continue;

	d_scale = (objects[i_plot].z_projected) / delta;

	icon_size = objects[i_plot].icon_size * (1.+d_scale);
	min_size = 0.5*objects[i_plot].icon_size;

	if ( icon_size > sim_set->icon_size_max ) icon_size = sim_set->icon_size_max;
	if ( icon_size < min_size ) icon_size = min_size;

	if ( d_scale < 0. ){
		brightness = 1.+d_scale;
		if ( brightness < 0.5 ) brightness = 0.5;
		if ( brightness > 1. ) brightness = 1.;
	}
	else{
		brightness = 1.;
	}

	render_icon_size_brightness(renderer, &objects[i_plot], sim_set, icon_size, brightness);
}

}

