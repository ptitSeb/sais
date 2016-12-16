#ifndef DEMO_VERSION

#include <stdio.h>
#ifdef LINUX
#include <dirent.h>
#else
#include <io.h>
#endif
#include <malloc.h>
#include <memory.h>
#include <string.h>


#include "typedefs.h"
#include "is_fileio.h"
#include "iface_globals.h"
#include "gfx.h"
#include "interface.h"
#include "snd.h"


#define MAX_MODDIRS 64
#define MOD_INTERFACE_COLOR 11

typedef struct _t_moddir
{
	char name[32];
	char dir[224];
} t_moddir;

t_moddir *moddirs;
int n_moddirs;

void modconfig_init()
{
	int x;
	int y;
#ifdef LINUX
	DIR *d;
	struct dirent *dir;
#else
	int handle;
	_finddata_t find;
#endif
	char tmps[256];
	FILE *fil;

	moddir[0] = 0;

	// allocate memory for mod names
	moddirs = (t_moddir*)calloc(MAX_MODDIRS, sizeof(t_moddir));
	n_moddirs = 0;

	// read mod names
#ifdef LINUX
	d = opendir("mods");
	if (d)
	{
		while ((dir = readdir(d)) != NULL)
		{
			if (n_moddirs < MAX_MODDIRS)
			if (dir->d_type == DT_DIR)
			{
				if (strcmp(dir->d_name, ".") && strcmp(dir->d_name, ".."))
				{
					sprintf(moddirs[n_moddirs].dir, "mods/%s/", dir->d_name);
					sprintf(moddirs[n_moddirs].name, dir->d_name);
					n_moddirs++;
				}
			}
		}

		closedir(d);
	}

#else
	handle = _findfirst("mods/*.*", &find);
	if (handle != -1)
	{
		x = handle;
		while (x != -1)
		{
			if (n_moddirs < MAX_MODDIRS)
			if (find.attrib & _A_SUBDIR)
			{
				if (strcmp(find.name, ".") && strcmp(find.name, ".."))
				{
					sprintf(moddirs[n_moddirs].dir, "mods/%s/", find.name);
					sprintf(moddirs[n_moddirs].name, find.name);
					n_moddirs++;
				}
			}
			x = _findnext(handle, &find);
		}
		_findclose(handle);
	}
#endif
	if (n_moddirs > 1)
	{
		for (x = 1; x < n_moddirs; x++)
		{
			y = x;
			while (y > 0 && strcmp(moddirs[y].name, moddirs[y-1].name) < 0)
			{
				strcpy(tmps, moddirs[y].name);
				strcpy(moddirs[y].name, moddirs[y-1].name);
				strcpy(moddirs[y-1].name, tmps);
				strcpy(tmps, moddirs[y].dir);
				strcpy(moddirs[y].dir, moddirs[y-1].dir);
				strcpy(moddirs[y-1].dir, tmps);
				y--;
			}
		}
	}

	fil = myopen("graphics/palette.dat", "rb");
	fread(globalpal, 1, 768, fil);
	fclose(fil);
	memcpy(currentpal, globalpal, 768);

	Load_WAV("sounds/beep_wait.wav",0);
	s_volume = 100;
	interface_init();
}

void modconfig_deinit()
{
	interface_deinit();
	Delete_Sound(0);

	free(moddirs);
	n_moddirs = 0;
}

int modconfig_main()
{
	int32 t, t0;
	int32 c, mc, mx, my;
	int32 bx, by, h;
	int32 x, y;
	int32 msel, mscr;
	t_ik_image *backy;
	int32 mode;
	int end;

	modconfig_init();

	if (!n_moddirs)
	{
		modconfig_deinit();
		return 1;
	}

	msel = 0; mscr = 0; mode = 0;
	backy = ik_load_pcx("graphics/starback.pcx", NULL);

	start_ik_timer(0, 20); t = 0;
	end = 0;
	while (!end && !must_quit)
	{
		ik_eventhandler();
		t0 = t; t = get_ik_timer(0);
		c = ik_inkey();
		mc = ik_mclick();
		mx = ik_mouse_x;
		my = ik_mouse_y;

		if (c == 13 || c == 32)
		{
			end = 1;
			if (mode == 1)
			{
				if (strcmp(moddir, moddirs[msel].dir))
				{	
					sprintf(moddir, moddirs[msel].dir);
					end = 1; 
				}
			}
		}

		if (mc & 1)
		{
			switch(mode)
			{
				case 0:	// main menu
				if (mx > bx+48 && mx < bx+208 && my > by+35 && my < by+108)
				{
					c = (my-(by+36))/24;
					if (c == 0)
					{
						moddir[0] = 0;
						end = 1;
						Play_SoundFX(0, 0, 100);
					}
					else if (c == 1)
					{
						mode = 1;
						Play_SoundFX(0, 0, 100);
					}
					else if (c == 2)
					{
						must_quit = 1;
						Play_SoundFX(0, 0, 100);
					}
				}
				break;

				case 1:	// mods
				if (mx > bx+16 && mx < bx+224 && my > by+35 && my < by+112)
				{
					c = mscr + (my-(by+36))/8;
					if (c >= 0 && c < n_moddirs)
					{
						msel = c;
						Play_SoundFX(0, 0, 100);
					}
					else
					{
						for (y = 0; y < n_moddirs; y++)
							if (!strcmp(moddirs[y].dir, moddir))
								msel = y;
						Play_SoundFX(0, 0, 100);
					}				
				}
				if (mx > bx+16 && mx < bx+80 && my > by+h-32 && my < by+h-16)
				{	
					mode = 0; 
					Play_SoundFX(0, 0, 100);
				}
				if (mx > bx+176 && mx < bx+240 && my > by+h-32 && my < by+h-16)
				if (strcmp(moddir, moddirs[msel].dir))
				{	
					sprintf(moddir, moddirs[msel].dir);
					Play_SoundFX(0, 0, 100);
					end = 1; 
				}
				break;

				default: ;
			}
		}

		if (t > t0)
		{
			prep_screen();
			ik_copybox(backy, screen, 0, 0, 639, 479, 0, 0);
#ifdef BIGGER
#define WIDE 280
#define DECAL (WIDE-256)/2
			t_ik_font* font_backup = font_6x8;
			font_6x8 = font_7x8;
#else
#define WIDE 256
#define DECAL 0
#endif
			switch (mode)
			{
				case 0:	// main menu
				bx = 192; by = 164; h = 128;
				interface_drawborder(screen, bx-DECAL, by, bx+WIDE, by+h, 1, 
						MOD_INTERFACE_COLOR, "Strange Adventures in Infinite Space");

				interface_drawbutton(screen, bx+48, by+40, 160, MOD_INTERFACE_COLOR, "STANDARD GAME");
				interface_drawbutton(screen, bx+48, by+64, 160, MOD_INTERFACE_COLOR, "MODS");
				interface_drawbutton(screen, bx+48, by+88, 160, MOD_INTERFACE_COLOR, "EXIT");
				break;

				case 1:
				bx = 192; by = 148; h = 160;
				interface_drawborder(screen, bx-DECAL, by, bx+WIDE, by+h, 1, 
						MOD_INTERFACE_COLOR, "Strange Adventures in Infinite Space");

				y = 0;
				for (x = 0; x < n_moddirs; x++)
					if (!strcmp(moddirs[x].dir, moddir))
						y = x;

				ik_print(screen, font_6x8, bx+16, by+22, 0, "Select a mod", moddirs[y].name);
				interface_thinborder(screen, bx+16, by+32, bx+240, by+120, MOD_INTERFACE_COLOR, 0);
				
				for (x = 0; x < n_moddirs; x++)
				{
					y = x - mscr;
					if (x == msel)
						ik_drawbox(screen, bx+16, by+35+y*8, bx+239, by+43+y*8, 3); //STARMAP_INTERFACE_COLOR*16+4);
					ik_print(screen, font_6x8, bx+20, by+36+y*8, 3*(msel==x), moddirs[x].name);
				}

				if (n_moddirs > 10)
				{
					ik_dsprite(screen, bx+228, by+36, spr_IFarrows->spr[5], 2+(MOD_INTERFACE_COLOR<<8));
					ik_dsprite(screen, bx+228, by+108, spr_IFarrows->spr[4], 2+(MOD_INTERFACE_COLOR<<8));
					interface_drawslider(screen, bx + 228, by + 44, 1, 64, n_moddirs-10, mscr, MOD_INTERFACE_COLOR);
				}
				interface_thinborder(screen, bx+16, by+32, bx+240, by+120, MOD_INTERFACE_COLOR);

				interface_drawbutton(screen, bx+16, by+h-32, 64, MOD_INTERFACE_COLOR, "CANCEL");
				interface_drawbutton(screen, bx+WIDE-80, by+h-32, 64, MOD_INTERFACE_COLOR, "RUN MOD");
				break;

				default: ;
			}
#ifdef BIGGER
			font_6x8 = font_backup;
#endif
			update_palette();
			ik_blit();	
		}
	}
#undef WIDE
	prep_screen();
	ik_drawbox(screen, 0, 0, 639, 479, 0);
	ik_blit();

	del_image(backy);
	modconfig_deinit();

	return end;
}




#endif
