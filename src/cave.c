/* File: cave.c */

/* distance, LOS (and targetting), destruction of a square, legal object
 * and monster codes, hallucination, code for dungeon display, memorization
 * of objects and features, small-scale dungeon maps,  and management,
 * magic mapping, wizard light the dungeon, forget the dungeon, the pro-
 * jection code, disturb player, check for quest level.
 *
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies.  Other copyrights may also apply.
 */

#include "angband.h"



/*
 * Approximate Distance between two points.
 *
 * When either the X or Y component dwarfs the other component,
 * this function is almost perfect, and otherwise, it tends to
 * over-estimate about one grid per fifteen grids of distance.
 *
 * Algorithm: hypot(dy,dx) = max(dy,dx) + min(dy,dx) / 2
 */
sint distance(int y1, int x1, int y2, int x2)
{
  int ay, ax;
  
  /* Find the absolute y/x distance components */
  ay = (y1 > y2) ? (y1 - y2) : (y2 - y1);
  ax = (x1 > x2) ? (x1 - x2) : (x2 - x1);
  
  /* Hack -- approximate the distance */
  return ((ay > ax) ? (ay + (ax>>1)) : (ax + (ay>>1)));
}


/*
 * A simple, fast, integer-based line-of-sight algorithm.  By Joseph Hall,
 * 4116 Brewster Drive, Raleigh NC 27606.  Email to jnh@ecemwl.ncsu.edu.
 *
 * This function returns TRUE if a "line of sight" can be traced from the
 * center of the grid (x1,y1) to the center of the grid (x2,y2), with all
 * of the grids along this path (except for the endpoints) being non-wall
 * grids, that are also not trees or rubble.  Actually, the "chess knight 
 * move" situation is handled by some special case code which allows the 
 * grid diagonally next to the player to be obstructed, because this 
 * yields better gameplay semantics.  This algorithm is totally reflexive, 
 * except for "knight move" situations.
 *
 * Because this function uses (short) ints for all calculations, overflow
 * may occur if dx and dy exceed 90.
 *
 * Once all the degenerate cases are eliminated, we determine the "slope"
 * ("m"), and we use special "fixed point" mathematics in which we use a
 * special "fractional component" for one of the two location components
 * ("qy" or "qx"), which, along with the slope itself, are "scaled" by a
 * scale factor equal to "abs(dy*dx*2)" to keep the math simple.  Then we
 * simply travel from start to finish along the longer axis, starting at
 * the border between the first and second tiles (where the y offset is
 * thus half the slope), using slope and the fractional component to see
 * when motion along the shorter axis is necessary.  Since we assume that
 * vision is not blocked by "brushing" the corner of any grid, we must do
 * some special checks to avoid testing grids which are "brushed" but not
 * actually "entered".
 *
 * Angband three different "line of sight" type concepts, including this
 * function (which is used almost nowhere), the "project()" method (which
 * is used for determining the paths of projectables and spells and such),
 * and the "update_view()" concept (which is used to determine which grids
 * are "viewable" by the player, which is used for many things, such as
 * determining which grids are illuminated by the player's torch, and which
 * grids and monsters can be "seen" by the player, etc).
 */
bool los(int y1, int x1, int y2, int x2)
{
  /* Delta */
  int dx, dy;
  
  /* Absolute */
  int ax, ay;
  
  /* Signs */
  int sx, sy;
  
  /* Fractions */
  int qx, qy;
  
  /* Scanners */
  int tx, ty;
  
  /* Scale factors */
  int f1, f2;
  
  /* Slope, or 1/Slope, of LOS */
  int m;
  
  
  /* Extract the offset */
  dy = y2 - y1;
  dx = x2 - x1;
  
  /* Extract the absolute offset */
  ay = ABS(dy);
  ax = ABS(dx);
  
  
  /* Handle adjacent (or identical) grids */
  if ((ax < 2) && (ay < 2)) return (TRUE);
  
  
  /* Directly South/North */
  if (!dx)
    {
      /* South -- check for walls */
      if (dy > 0)
	{
	  for (ty = y1 + 1; ty < y2; ty++)
	    {
	      if (!cave_floor_bold(ty, x1)) return (FALSE);
	    }
	}
      
      /* North -- check for walls */
      else
	{
	  for (ty = y1 - 1; ty > y2; ty--)
	    {
	      if (!cave_floor_bold(ty, x1)) return (FALSE);
	    }
	}
      
      /* Assume los */
      return (TRUE);
    }
  
  /* Directly East/West */
  if (!dy)
    {
      /* East -- check for walls */
      if (dx > 0)
	{
	  for (tx = x1 + 1; tx < x2; tx++)
	    {
	      if (!cave_floor_bold(y1, tx)) return (FALSE);
	    }
	}
      
      /* West -- check for walls */
      else
	{
	  for (tx = x1 - 1; tx > x2; tx--)
	    {
	      if (!cave_floor_bold(y1, tx)) return (FALSE);
	    }
	}
      
      /* Assume los */
      return (TRUE);
    }
  
  
  /* Extract some signs */
  sx = (dx < 0) ? -1 : 1;
  sy = (dy < 0) ? -1 : 1;
  
  
  /* Vertical "knights" */
  if (ax == 1)
    {
      if (ay == 2)
	{
	  if (cave_floor_bold(y1 + sy, x1)) return (TRUE);
	}
    }
  
  /* Horizontal "knights" */
  else if (ay == 1)
    {
      if (ax == 2)
	{
	  if (cave_floor_bold(y1, x1 + sx)) return (TRUE);
	}
    }
  
  
  /* Calculate scale factor div 2 */
  f2 = (ax * ay);
  
  /* Calculate scale factor */
  f1 = f2 << 1;

  
  /* Travel horizontally */
  if (ax >= ay)
    {
      /* Let m = dy / dx * 2 * (dy * dx) = 2 * dy * dy */
      qy = ay * ay;
      m = qy << 1;
      
      tx = x1 + sx;
      
      /* Consider the special case where slope == 1. */
      if (qy == f2)
	{
	  ty = y1 + sy;
	  qy -= f1;
	}
      else
	{
	  ty = y1;
	}
      
      /* Note (below) the case (qy == f2), where */
      /* the LOS exactly meets the corner of a tile. */
      while (x2 - tx)
	{
	  if (!cave_floor_bold(ty, tx)) return (FALSE);
	  
	  qy += m;
	  
	  if (qy < f2)
	    {
	      tx += sx;
	    }
	  else if (qy > f2)
	    {
	      ty += sy;
	      if (!cave_floor_bold(ty, tx)) return (FALSE);
	      qy -= f1;
	      tx += sx;
	    }
	  else
	    {
	      ty += sy;
	      qy -= f1;
	      tx += sx;
	    }
	}
    }
  
  /* Travel vertically */
  else
    {
		/* Let m = dx / dy * 2 * (dx * dy) = 2 * dx * dx */
      qx = ax * ax;
      m = qx << 1;
      
      ty = y1 + sy;
      
      if (qx == f2)
	{
	  tx = x1 + sx;
	  qx -= f1;
	}
      else
	{
	  tx = x1;
	}
      
      /* Note (below) the case (qx == f2), where */
      /* the LOS exactly meets the corner of a tile. */
      while (y2 - ty)
	{
	  if (!cave_floor_bold(ty, tx)) return (FALSE);
	  
	  qx += m;
	  
	  if (qx < f2)
	    {
	      ty += sy;
	    }
	  else if (qx > f2)
	    {
	      tx += sx;
	      if (!cave_floor_bold(ty, tx)) return (FALSE);
	      qx -= f1;
	      ty += sy;
	    }
	  else
	    {
	      tx += sx;
	      qx -= f1;
	      ty += sy;
	    }
	}
    }
  
  /* Assume los */
  return (TRUE);
}




/*
 * Returns true if the player's grid is dark
 * Players with the UNLIGHT ability don't need light and always
 * return false.
 */
bool no_lite(void)
{
  int py = p_ptr->py;
  int px = p_ptr->px;
  
  if (check_ability(SP_UNLIGHT) || p_ptr->darkness) return(FALSE);
  
  return (!player_can_see_bold(py, px));
}




/*
 * Determine if a given location may be "destroyed"
 *
 * Used by destruction spells, and for placing stairs, etc.
 */
bool cave_valid_bold(int y, int x)
{
  s16b this_o_idx, next_o_idx = 0;
  feature_type *f_ptr = &f_info[cave_feat[y][x]];  

  /* Forbid perma-grids */
  if (f_ptr->flags & TF_PERMANENT) return (FALSE);
  
  /* Check objects */
  for (this_o_idx = cave_o_idx[y][x]; this_o_idx; this_o_idx = next_o_idx)
    {
      object_type *o_ptr;
      
      /* Acquire object */
      o_ptr = &o_list[this_o_idx];
      
      /* Acquire next object */
      next_o_idx = o_ptr->next_o_idx;
      
      /* Forbid artifact grids */
      if (artifact_p(o_ptr)) return (FALSE);
    }
  
  /* Accept */
  return (TRUE);
}


/* 
 * Table of breath colors.  Must match listings in a single set of 
 * monster spell flags.
 *
 * The value "255" is special.  Monsters with that kind of breath 
 * may be any color.
 */
static byte breath_to_attr[32][2] = 
  {
    {  0,  0 },
    {  0,  0 },
    {  0,  0 },
    {  0,  0 },
    {  0,  0 },
    {  0,  0 },
    {  0,  0 },
    {  0,  0 },
    {  TERM_SLATE, TERM_L_DARK },       /* RF4_BRTH_ACID */
    {  TERM_BLUE,  TERM_L_BLUE },       /* RF4_BRTH_ELEC */
    {  TERM_RED,  TERM_L_RED },         /* RF4_BRTH_FIRE */
    {  TERM_WHITE,  TERM_L_WHITE },     /* RF4_BRTH_COLD */
    {  TERM_GREEN,  TERM_L_GREEN },     /* RF4_BRTH_POIS */
    {  TERM_ORANGE,  TERM_RED },        /* RF4_BRTH_PLAS */
    {  TERM_YELLOW,  TERM_ORANGE },     /* RF4_BRTH_LITE */
    {  TERM_L_DARK,  TERM_SLATE },      /* RF4_BRTH_DARK */
    {  TERM_L_UMBER,  TERM_UMBER },     /* RF4_BRTH_CONFU */
    {  TERM_YELLOW,  TERM_L_UMBER },    /* RF4_BRTH_SOUND */
    {  TERM_UMBER,  TERM_L_UMBER },     /* RF4_BRTH_SHARD */
    {  TERM_L_WHITE,  TERM_SLATE },     /* RF4_BRTH_INER */
    {  TERM_L_WHITE,  TERM_SLATE },     /* RF4_BRTH_GRAV */
    {  TERM_UMBER,  TERM_L_UMBER },     /* RF4_BRTH_FORCE */
    {  TERM_L_RED,  TERM_VIOLET },      /* RF4_BRTH_NEXUS */
    {  TERM_L_GREEN,  TERM_GREEN },     /* RF4_BRTH_NETHR */
    {  255,  255 },   /* (any color) */ /* RF4_BRTH_CHAOS */
    {  TERM_VIOLET,  TERM_VIOLET },     /* RF4_BRTH_DISEN */
    {  TERM_L_BLUE,  TERM_L_BLUE },     /* RF4_BRTH_TIME */
    {  TERM_BLUE,     TERM_SLATE },     /* RF4_BRTH_STORM */
    {  TERM_RED,      TERM_GREEN },     /* RF4_BRTH_DFIRE */
    {  TERM_WHITE,  TERM_L_WHITE },     /* RF4_BRTH_ICE */
    {  0,  0 },     /*  */
    {  0,  0 }      /*  */
  };


/*
 * Multi-hued monsters shimmer acording to their breaths.
 *
 * If a monster has only one kind of breath, it uses both colors 
 * associated with that breath.  Otherwise, it just uses the first 
 * color for any of its breaths.
 *
 * If a monster does not breath anything, it can be any color.
 */
static byte multi_hued_attr(monster_race *r_ptr)
{
  byte allowed_attrs[15];
  
  int i, j;
  
  int stored_colors = 0;
  int breaths = 0;
  int first_color = 0;
  int second_color = 0;
  
  
  /* Monsters with no ranged attacks can be any color */
  if (!r_ptr->freq_ranged) return (randint(15));
  
  /* Check breaths */
  for (i = 0; i < 32; i++)
    {
      bool stored = FALSE;
      
      /* Don't have that breath */
      if (!(r_ptr->flags4 & (1L << i))) continue;
      
      /* Get the first color of this breath */
      first_color = breath_to_attr[i][0];
      
      /* Breath has no color associated with it */
      if (first_color == 0) continue;
      
      /* Monster can be of any color */
      if (first_color == 255) return (randint(15));
      
      
      /* Increment the number of breaths */
      breaths++;
      
      /* Monsters with lots of breaths may be any color. */
      if (breaths == 6) return (randint(15));
      
      
      /* Always store the first color */
      for (j = 0; j < stored_colors; j++)
	{
	  /* Already stored */
	  if (allowed_attrs[j] == first_color) stored = TRUE;
	}
      if (!stored)
	{
	  allowed_attrs[stored_colors] = first_color;
	  stored_colors++;
	}
      
      /* 
       * Remember (but do not immediately store) the second color 
       * of the first breath.
       */
      if (breaths == 1)
	{
	  second_color = breath_to_attr[i][1];
	}
    }
  
  /* Monsters with no breaths may be of any color. */
  if (breaths == 0) return (randint(15));
  
  /* If monster has one breath, store the second color too. */
  if (breaths == 1)
    {
      allowed_attrs[stored_colors] = second_color;
      stored_colors++;
    }
  
  /* Pick a color at random */
  return (allowed_attrs[rand_int(stored_colors)]);
}

/*
 * Hack -- Legal monster codes
 */
static const char image_monster_hack[] = \
"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

/*
 * Hack -- Hallucinatory monster
 */
static u16b image_monster(void)
{
  byte a;
  char c;
  
  /* Random symbol from set above (not including final nul) */
  c = image_monster_hack[rand_int(sizeof(image_monster_hack) - 1)];
  
  /* Random color */
  a = randint(15);
  
  /* Encode */
  return (PICT(a,c));
}


/*
 * Hack -- Legal object codes
 */
static const char image_object_hack[] = \
"?/|\\\"!$()_-=[]{},~"; /* " */

/*
 * Hack -- Hallucinatory object
 */
static u16b image_object(void)
{
  byte a;
  char c;
  
  /* Random symbol from set above (not including final nul) */
  c = image_object_hack[rand_int(sizeof(image_object_hack) - 1)];
  
  /* Random color */
  a = randint(15);
  
  /* Encode */
  return (PICT(a,c));
}


/*
 * Hack -- Random hallucination
 */
static u16b image_random(void)
{
  /* Normally, assume monsters */
  if (rand_int(100) < 75)
    {
      return (image_monster());
    }
  
  /* Otherwise, assume objects */
  else
    {
      return (image_object());
    }
}



/*
 * The 16x16 tile of the terrain supports lighting
 */
bool feat_supports_lighting(byte feat)
{
  if ((feat >= FEAT_LAVA) && (feat <= FEAT_WEB))
    return TRUE;
  
  switch (feat)
    {
    case FEAT_FLOOR:
    case FEAT_INVIS:
    case FEAT_SECRET:
    case FEAT_MAGMA:
    case FEAT_QUARTZ:
    case FEAT_MAGMA_H:
    case FEAT_QUARTZ_H:
    case FEAT_WALL_EXTRA:
    case FEAT_WALL_INNER:
    case FEAT_WALL_OUTER:
    case FEAT_WALL_SOLID:
    case FEAT_PERM_EXTRA:
    case FEAT_PERM_INNER:
    case FEAT_PERM_OUTER:
    case FEAT_PERM_SOLID:
      return TRUE;
    default:
      return FALSE;
    }
}


/*
 * Extract the attr/char to display at the given (legal) map location
 *
 * Note that this function, since it is called by "lite_spot()" which
 * is called by "update_view()", is a major efficiency concern.
 *
 * Basically, we examine each "layer" of the world (terrain, objects,
 * monsters/players), from the bottom up, extracting a new attr/char
 * if necessary at each layer, and defaulting to "darkness".  This is
 * not the fastest method, but it is very simple, and it is about as
 * fast as it could be for grids which contain no "marked" objects or
 * "visible" monsters.
 *
 * We apply the effects of hallucination during each layer.  Objects will
 * always appear as random "objects", monsters will always appear as random
 * "monsters", and normal grids occasionally appear as random "monsters" or
 * "objects", but note that these random "monsters" and "objects" are really
 * just "colored ascii symbols" (which may look silly on some machines).
 *
 * The hallucination functions avoid taking any pointers to local variables
 * because some compilers refuse to use registers for any local variables
 * whose address is taken anywhere in the function.
 *
 * As an optimization, we can handle the "player" grid as a special case.
 *
 * Note that the memorization of "objects" and "monsters" is not related
 * to the memorization of "terrain".  This allows the player to memorize
 * the terrain of a grid without memorizing any objects in that grid, and
 * to detect monsters without detecting anything about the terrain of the
 * grid containing the monster.
 *
 * The fact that all interesting "objects" and "terrain features" are
 * memorized as soon as they become visible for the first time means
 * that we only have to check the "CAVE_SEEN" flag for "boring" grids.
 *
 * Note that bizarre things must be done when the "attr" and/or "char"
 * codes have the "high-bit" set, since these values are used to encode
 * various "special" pictures in some versions, and certain situations,
 * such as "multi-hued" or "clear" monsters, cause the attr/char codes
 * to be "scrambled" in various ways.
 *
 * Note that the "zero" entry in the feature/object/monster arrays are
 * used to provide "special" attr/char codes, with "monster zero" being
 * used for the player attr/char, "object zero" being used for the "pile"
 * attr/char, and "feature zero" being used for the "darkness" attr/char.
 *
 * Note that eventually we may want to use the "&" symbol for embedded
 * treasure, and use the "*" symbol to indicate multiple objects, but
 * currently, we simply use the attr/char of the first "marked" object
 * in the stack, if any, and so "object zero" is unused.  XXX XXX XXX
 *
 * Note the assumption that doing "x_ptr = &x_info[x]" plus a few of
 * "x_ptr->xxx", is quicker than "x_info[x].xxx", even if "x" is a fixed
 * constant.  If this is incorrect then a lot of code should be changed.
 *
 *
 * Some comments on the "terrain" layer...
 *
 * Note that "boring" grids (floors, invisible traps, and any illegal grids)
 * are very different from "interesting" grids (all other terrain features),
 * and the two types of grids are handled completely separately.  The most
 * important distinction is that "boring" grids may or may not be memorized
 * when they are first encountered, and so we must use the "CAVE_SEEN" flag
 * to see if they are "see-able".
 *
 *
 * Some comments on the "terrain" layer (boring grids)...
 *
 * Note that "boring" grids are always drawn using the picture for "empty
 * floors", which is stored in "f_info[FEAT_FLOOR]".  Sometimes, special
 * lighting effects may cause this picture to be modified.
 *
 * Note that "invisible traps" are always displayed exactly like "empty
 * floors", which prevents various forms of "cheating", with no loss of
 * efficiency.  There are still a few ways to "guess" where traps may be
 * located, for example, objects will never fall into a grid containing
 * an invisible trap.  XXX XXX
 *
 * To determine if a "boring" grid should be displayed, we simply check to
 * see if it is either memorized ("CAVE_MARK"), or currently "see-able" by
 * the player ("CAVE_SEEN").  Note that "CAVE_SEEN" is now maintained by the
 * "update_view()" function.
 *
 * Note the "special lighting effects" which can be activated for "boring"
 * grids using the "view_special_lite" option, causing certain such grids
 * to be displayed using special colors (if they are normally "white").
 * If the grid is "see-able" by the player, we will use the normal "white"
 * (except that, if the "view_yellow_lite" option is set, and the grid
 * is *only* "see-able" because of the player's torch, then we will use
 * "yellow"), else if the player is "blind", we will use "dark gray",
 * else if the grid is not "illuminated", we will use "dark gray", else
 * if the "view_bright_lite" option is set, we will use "slate" (gray),
 * else we will use the normal "white".
 *
 *
 * Some comments on the "terrain" layer (non-boring grids)...
 *
 * Note the use of the "mimic" field in the "terrain feature" processing,
 * which allows any feature to "pretend" to be another feature.  This is
 * used to "hide" secret doors, and to make all "doors" appear the same,
 * and all "walls" appear the same, and "hidden" treasure stay hidden.
 * Note that it is possible to use this field to make a feature "look"
 * like a floor, but the "view_special_lite" flag only affects actual
 * "boring" grids.
 *
 * Since "interesting" grids are always memorized as soon as they become
 * "see-able" by the player ("CAVE_SEEN"), such a grid only needs to be
 * displayed if it is memorized ("CAVE_MARK").  Most "interesting" grids
 * are in fact non-memorized, non-see-able, wall grids, so the fact that
 * we do not have to check the "CAVE_SEEN" flag adds some efficiency, at
 * the cost of *forcing* the memorization of all "interesting" grids when
 * they are first seen.  Since the "CAVE_SEEN" flag is now maintained by
 * the "update_view()" function, this efficiency is not as significant as
 * it was in previous versions, and could perhaps be removed.
 *
 * Note the "special lighting effects" which can be activated for "wall"
 * grids using the "view_granite_lite" option, causing certain such grids
 * to be displayed using special colors (if they are normally "white").
 * If the grid is "see-able" by the player, we will use the normal "white"
 * else if the player is "blind", we will use "dark gray", else if the
 * "view_bright_lite" option is set, we will use "slate" (gray), else we
 * will use the normal "white".
 *
 * Note that "wall" grids are more complicated than "boring" grids, due to
 * the fact that "CAVE_GLOW" for a "wall" grid means that the grid *might*
 * be glowing, depending on where the player is standing in relation to the
 * wall.  In particular, the wall of an illuminated room should look just
 * like any other (dark) wall unless the player is actually inside the room.
 *
 * Thus, we do not support as many visual special effects for "wall" grids
 * as we do for "boring" grids, since many of them would give the player
 * information about the "CAVE_GLOW" flag of the wall grid, in particular,
 * it would allow the player to notice the walls of illuminated rooms from
 * a dark hallway that happened to run beside the room.
 *
 *
 * Some comments on the "object" layer...
 *
 * Currently, we do nothing with multi-hued objects, because there are
 * not any.  If there were, they would have to set "shimmer_objects"
 * when they were created, and then new "shimmer" code in "dungeon.c"
 * would have to be created handle the "shimmer" effect, and the code
 * in "cave.c" would have to be updated to create the shimmer effect.
 * This did not seem worth the effort.  XXX XXX
 *
 *
 * Some comments on the "monster"/"player" layer...
 *
 * Note that monsters can have some "special" flags, including "ATTR_MULTI",
 * which means their color changes, and "ATTR_CLEAR", which means they take
 * the color of whatever is under them, and "CHAR_CLEAR", which means that
 * they take the symbol of whatever is under them.  Technically, the flag
 * "CHAR_MULTI" is supposed to indicate that a monster looks strange when
 * examined, but this flag is currently ignored.  All of these flags are
 * ignored if the "avoid_other" option is set, since checking for these
 * conditions is expensive (and annoying) on some systems.
 *
 * Normally, players could be handled just like monsters, except that the
 * concept of the "torch lite" of others player would add complications.
 * For efficiency, however, we handle the (only) player first, since the
 * "player" symbol always "pre-empts" any other facts about the grid.
 *
 * The "hidden_player" efficiency option, which only makes sense with a
 * single player, allows the player symbol to be hidden while running.
 *
 * ToDo: The transformations for tile colors, or brightness for the 16x16
 * tiles should be handled differently.  One possibility would be to
 * extend feature_type with attr/char definitions for the different states.
 */
void map_info(int y, int x, byte *ap, char *cp, byte *tap, char *tcp)
{
  byte a;
  char c;
  
  byte feat;
  byte info;
  
  feature_type *f_ptr;
  
  object_type *o_ptr = &inventory[INVEN_LITE];

  s16b this_o_idx, next_o_idx = 0;
  
  s16b m_idx;
  
  s16b image = p_ptr->image;
  
  int floor_num = 0;
  
  /* Hack -- Assume that "new" means "Adam Bolt Tiles" (or DG - NRM) */
  bool graf_new = (use_graphics && (streq(ANGBAND_GRAF, "new") || 
				    streq(ANGBAND_GRAF, "david")));

  /* Monster/Player */
  m_idx = cave_m_idx[y][x];
  
  /* Feature */
  feat = cave_feat[y][x];
  
  /* Cave flags */
  info = cave_info[y][x];
  
  /* Hack -- rare random hallucination on non-outer walls */
  if (image && (feat < FEAT_PERM_SOLID) && (image_count-- <= 0))
    {
      int i;
      
      /* Display a random image, reset count. */
      image_count = randint(511);
      i = image_random();
      
      a = PICT_A(i);
      c = PICT_C(i);
    }
  
  /* Boring grids (floors, etc) */
  else if (feat <= FEAT_INVIS)
    {
      /* Memorized (or seen) floor */
      if ((info & (CAVE_MARK)) ||
	  (info & (CAVE_SEEN)))
	{
	  /* Get the floor feature */
	  f_ptr = &f_info[FEAT_FLOOR];
	  
	  /* Normal attr */
	  a = f_ptr->x_attr;
	  
	  /* Normal char */
	  c = f_ptr->x_char;
	  
	  /* Special lighting effects */
	  if (view_special_lite && ((a == TERM_WHITE) || graf_new))
	    {
	      /* Handle "seen" grids */
	      if (info & (CAVE_SEEN))
		{
		  /* Only lit by "torch" lite */
		  if (view_yellow_lite && !(info & (CAVE_GLOW)) && 
		      (!is_daylight))
		    {
		      if (arg_graphics == GRAPHICS_DAVID_GERVAIS)
			if ((check_ability(SP_UNLIGHT) || p_ptr->darkness) && 
			    (p_ptr->cur_lite <= 0))
			  {
			    /* "Dark radius" */
			  }
			else
			  {
			    c -= 1;
			  }
		      else if (graf_new)
			{
			  if ((check_ability(SP_UNLIGHT) || p_ptr->darkness) && 
			      (p_ptr->cur_lite <= 0))
			    {
			      /* "Dark radius" */
			    }
			  else
			    {
			      /* Use a brightly lit tile */
			      c += 2;
			    }
			}
		      else if (o_ptr->k_idx == 733)
			{
			  /* Hack for Lamp of Gwindor */
			  a = TERM_L_BLUE;
			}
		      else if ((check_ability(SP_UNLIGHT) || p_ptr->darkness) 
			       && (p_ptr->cur_lite <= 0))
			{
			  /* "Dark radius" */
			  a = TERM_L_DARK;
			}
		      else
			{
			  /* Use "yellow" */
			  a = TERM_YELLOW;
			}
		    }
		}
	      
	      /* Handle "blind" */
	      else if (p_ptr->blind)
		{
		  if (graf_new)
		    {
		      /* Use a dark tile */
		      c += 1;
		    }
		  else
		    {
		      /* Use "dark gray" */
		      a = TERM_L_DARK;
		    }
		}
	      
	      /* Handle "dark" grids */
	      else if (!(info & (CAVE_GLOW)))
		{
		  if (graf_new)
		    {
		      /* Use a dark tile */
		      c += 1;
		    }
		  else
		    {
		      /* Use "dark gray" */
		      a = TERM_L_DARK;
		    }
		}
	      
	      /* Handle "view_bright_lite" */
	      else if (view_bright_lite)
		{
		  if (graf_new)
		    {
		      /* Use a dark tile */
		      c += 1;
		    }
		  else
		    {
		      /* Use "gray" */
		      a = TERM_SLATE;
		    }
		}
	    }
	}
      
      /* Unknown */
      else
	{
	  /* Get the darkness feature */
	  f_ptr = &f_info[FEAT_NONE];
	  
	  /* Normal attr */
	  a = f_ptr->x_attr;
	  
	  /* Normal char */
	  c = f_ptr->x_char;
	}
    }
  
  /* Special terrain (trees etc) */
  else if (feat >= FEAT_MIN_SPECIAL)
    {
      /* Memorized (or seen) floor */
      if ((info & (CAVE_MARK)) ||
	  (info & (CAVE_SEEN)))
	{
	  /* Get the floor feature */
	  f_ptr = &f_info[feat];
	  
	  /* Normal attr */
	  a = f_ptr->x_attr;
	  
	  /* Normal char */
	  c = f_ptr->x_char;
	  
	  /* Special lighting effects */
	  if (view_special_lite)
	    {
	      /* Handle "seen" grids */
	      if (info & (CAVE_SEEN))
		{
		  /* Only lit by "torch" lite */
		  if (view_yellow_lite && !(info & (CAVE_GLOW))
		      && (a == TERM_WHITE) && (!is_daylight))
		    {
		      if (arg_graphics == GRAPHICS_DAVID_GERVAIS)
			if ((check_ability(SP_UNLIGHT) || p_ptr->darkness) && 
			    (p_ptr->cur_lite <= 0))
			  {
			    /* "Dark radius" */
			  }
			else
			  {
			    c -= 1;
			  }
		      else if (graf_new)
			{
			  if ((check_ability(SP_UNLIGHT) || p_ptr->darkness) && 
			      (p_ptr->cur_lite <= 0))
			    {
			      /* "Dark radius" */
			    }
			  else
			    {
			      /* Use a brightly lit tile */
			      c += 2;
			    }
			}
		      else if (o_ptr->k_idx == 733)
			{
			  /* Hack for Lamp of Gwindor */
			  a = TERM_L_BLUE;
			}
		      else if ((check_ability(SP_UNLIGHT) || p_ptr->darkness) 
			       && (p_ptr->cur_lite <= 0))
			{
			  /* "Dark radius" */
			  a = TERM_L_DARK;
			}
		      else
			{
			  /* Use "yellow" */
			  a = TERM_YELLOW;
			}
		    }
		}
	      
	      /* Handle "blind" */
	      else if (p_ptr->blind)
		{
		  if (graf_new)
		    {
		      /* Use a dark tile */
		      c += 1;
		    }
		  else
		    {
		      /* Use "dark gray" */
		      a = TERM_L_DARK;
		    }
		}
	      
	      /* Handle "dark" grids */
	      else if (!(info & (CAVE_GLOW)))
		{
		  if (graf_new)
		    {
		      /* Use a dark tile */
		      c += 1;
		    }
		  else
		    {
		      /* Use "dark gray" */
		      a = TERM_L_DARK;
		    }
		}
	      
	      /* Handle "view_bright_lite" */
	      else if (view_bright_lite)
		{
		  if (graf_new)
		    {
		      /* Use a dark tile */
		      c += 1;
		    }
		  else
		    {
		      /* Use "gray" */
		      a = TERM_SLATE;
		    }
		}
	    }
	}
      
      /* Unknown */
      else
	{
	  /* Get the darkness feature */
	  f_ptr = &f_info[FEAT_NONE];
	  
	  /* Normal attr */
	  a = f_ptr->x_attr;
	  
	  /* Normal char */
	  c = f_ptr->x_char;
	}
    }
  
  /* Interesting grids (non-floors) */
  else
    {
      /* Memorized grids */
      if (info & (CAVE_MARK))
	{
	  /* Apply "mimic" field */
	  feat = f_info[feat].mimic;
	  
	  /* Get the feature */
	  f_ptr = &f_info[feat];
	  
	  /* Normal attr */
	  a = f_ptr->x_attr;
	  
	  /* Normal char */
	  c = f_ptr->x_char;
	  
	  /* Special lighting effects (walls only) */
	  if (view_granite_lite && feat_supports_lighting(feat))
	      {
	      /* Handle "seen" grids */
	      if (info & (CAVE_SEEN))
		{
		  if (graf_new)
		    {
		      /* Use a lit tile */
		    }
		  else
		    {
		      /* Use "white" */
		    }
		}
	      
	      /* Handle "blind" */
	      else if (p_ptr->blind)
		{
		  if (graf_new)
		    {
		      /* Use a dark tile */
		      c += 1;
		    }
		  else
		    {
		      /* Use "dark gray" */
		      a = TERM_L_DARK;
		    }
		}
	      
	      /* Handle "view_bright_lite" */
	      else if (view_bright_lite)
		{
		  if (graf_new)
		    {
		      /* Use a lit tile */
		      c += 1;
		    }
		  else
		    {
		      /* Use "gray" */
		      a = TERM_SLATE;
		    }
		}
	      else
		{
		  if (graf_new)
		    {
		      /* Use a brightly lit tile */
		      c += 2;
		    }
		  else
		    {
		      /* Use "white" */
		    }
		}
	    }
	}
      
      /* Unknown */
      else
	{
	  /* Get the darkness feature */
	  f_ptr = &f_info[FEAT_NONE];
	  
	  /* Normal attr */
	  a = f_ptr->x_attr;
	  
	  /* Normal char */
	  c = f_ptr->x_char;
	}
    }
  
  /* Save the terrain info for the transparency effects */
  (*tap) = a;
  (*tcp) = c;
  
  /* Objects */
  for (this_o_idx = cave_o_idx[y][x]; this_o_idx; this_o_idx = next_o_idx)
    {
      object_type *o_ptr;
      
      /* Hack to not display objects in trees */
      if ((cave_feat[y][x] == FEAT_TREE) || (cave_feat[y][x] == FEAT_TREE2) || 
	  (cave_feat[y][x] == FEAT_RUBBLE))
	break;
	      
      /* Get the object */
      o_ptr = &o_list[this_o_idx];
      
      /* Get the next object */
      next_o_idx = o_ptr->next_o_idx;
      
      /* Memorized objects */
      if (o_ptr->marked && !squelch_hide_item(o_ptr))
	{
	  /* Hack -- object hallucination */
	  if (image)
	    {
	      int i = image_object();
	      
	      a = PICT_A(i);
	      c = PICT_C(i);
	      
	      break;
	    }
	  
	  /* Normal attr */
	  a = object_attr(o_ptr);
	  
	  /* Normal char */
	  c = object_char(o_ptr);
	  
	  /* First marked object */
	  if (!show_piles) break;
	  
	  /* Special stack symbol */
	  if (++floor_num > 1)
	    {
	      object_kind *k_ptr;
	      
	      /* Get the "pile" feature */
	      k_ptr = &k_info[0];
	      
	      /* Normal attr */
	      a = k_ptr->x_attr;
	      
	      /* Normal char */
	      c = k_ptr->x_char;
	      
	      break;
	    }
	}
    }
  
  
  /* Monsters */
  if (m_idx > 0)
    {
      monster_type *m_ptr = &m_list[m_idx];
      
      /* Visible monster */
      if (m_ptr->ml)
	{
	  monster_race *r_ptr = &r_info[m_ptr->r_idx];
	  
	  byte da;
	  char dc;
	  
	  /* Desired attr */
	  da = r_ptr->x_attr;
	  
	  /* Desired char */
	  dc = r_ptr->x_char;
	  
	  /* Hack -- monster hallucination */
	  if (image)
	    {
	      int i = image_monster();
	      
	      a = PICT_A(i);
	      c = PICT_C(i);
	    }
	  
	  /* Ignore weird codes */
	  else if (avoid_other)
	    {
	      /* Use attr */
	      a = da;
	      
	      /* Use char */
	      c = dc;
	    }
	  
	  /* Special attr/char codes */
	  else if ((da & 0x80) && (dc & 0x80))
	    {
	      /* Use attr */
	      a = da;
	      
	      /* Use char */
	      c = dc;
	    }
	  
	  /* Multi-hued monster */
	  else if (r_ptr->flags1 & (RF1_ATTR_MULTI))
	    {
	      /* Multi-hued attr */
	      a = multi_hued_attr(r_ptr);
	      
	      /* Normal char */
	      c = dc;
	    }
	  
	  /* Normal monster (not "clear" in any way) */
	  else if (!(r_ptr->flags1 & (RF1_ATTR_CLEAR | RF1_CHAR_CLEAR)))
	    {
	      /* Use attr */
	      a = da;
	      
	      /* Use char */
	      c = dc;
	    }
	  
	  /* Hack -- Bizarre grid under monster */
	  else if ((a & 0x80) || (c & 0x80))
	    {
	      /* Use attr */
	      a = da;
	      
	      /* Use char */
	      c = dc;
	    }
	  
	  /* Normal char, Clear attr, monster */
	  else if (!(r_ptr->flags1 & (RF1_CHAR_CLEAR)))
	    {
	      /* Normal char */
	      c = dc;
	    }
	  
	  /* Normal attr, Clear char, monster */
	  else if (!(r_ptr->flags1 & (RF1_ATTR_CLEAR)))
	    {
	      /* Normal attr */
	      a = da;
	    }
	}
    }
  
  /* Handle "player" */
  else if ((m_idx < 0) && !(p_ptr->running && hidden_player))
    {
      monster_race *r_ptr = &r_info[0];
      
      /* Get the "player" attr */
      /*  DSV:  I've chosen the following sequence of colors to indicate
	  the player's current HP.  There are colors are left over, but I
	  left them in this comment for easy reference, in the likely case
	  that I decide to change the order of color changes.
	  
	  TERM_WHITE		90-100% of HP remaining
	  TERM_YELLOW		70- 89% of HP remaining
	  TERM_ORANGE		50- 69% of HP remaining
	  TERM_L_RED		30- 49% of HP remaining
	  TERM_RED		 0- 29% of HP remaining
	  
	  
	  TERM_SLATE		_% of HP remaining
	  TERM_UMBER		_% of HP remaining
	  TERM_L_UMBER	_% of HP remaining
	  TERM_BLUE		-% of HP remaining
	  TERM_L_BLUE		-% of HP remaining
	  TERM_GREEN		-% of HP remaining
	  TERM_L_GREEN	-% of HP remaining
	  TERM_DARK		-% of HP remaining
	  TERM_L_DARK		-% of HP remaining
	  TERM_L_WHITE	-% of HP remaining
	  TERM_VIOLET		-% of HP remaining
      */
      
      if ((hp_changes_colour) && (arg_graphics == GRAPHICS_NONE))
	{
	  switch(p_ptr->chp * 10 / p_ptr->mhp)
	    {
	    case 10:
	    case  9:	a = TERM_WHITE  ;	break;
	    case  8:
	    case  7:	a = TERM_YELLOW ;	break;
	    case  6:
	    case  5:	a = TERM_ORANGE ;	break;
	    case  4:
	    case  3:	a = TERM_L_RED  ;	break;
	    case  2:
	    case  1:
	    case  0:	a = TERM_RED    ;	break;
	    default:	a = TERM_WHITE  ;	break;
	    }

	  /* if ((a == TERM_WHITE) && (check_ability(SP_UNLIGHT)) &&
	      (p_ptr->cur_lite <= 0))
	      a = TERM_L_DARK; */
	}
      
      else a = r_ptr->x_attr;
      
      /* Get the "player" char */
      c = r_ptr->x_char;
    }
  
#ifdef MAP_INFO_MULTIPLE_PLAYERS
  /* Players */
  else if (m_idx < 0)
#else /* MAP_INFO_MULTIPLE_PLAYERS */
    /* Handle "player" */
    else if ((m_idx < 0) && !(p_ptr->running && hidden_player))
#endif /* MAP_INFO_MULTIPLE_PLAYERS */
      {
	monster_race *r_ptr = &r_info[0];
	
	/* Get the "player" attr */
	a = r_ptr->x_attr;
	
	/* Get the "player" char */
	c = r_ptr->x_char;
      }
  
  /* Result */
  (*ap) = a;
  (*cp) = c;
}

/*
 * Repeat of map_info for use in screendumps.  A hack from NPP, but if Jeff
 * can't do better than this, I don't have a hope -NRM-
 */

void map_info_default(int y, int x, byte *ap, char *cp)
{
  byte a;
  char c;
  
  byte feat;
  byte info;
  
  feature_type *f_ptr;
  
  object_type *o_ptr = &inventory[INVEN_LITE];

  s16b this_o_idx, next_o_idx = 0;
  
  s16b m_idx;
  
  s16b image = p_ptr->image;
  
  int floor_num = 0;
  
  /* Monster/Player */
  m_idx = cave_m_idx[y][x];
  
  /* Feature */
  feat = cave_feat[y][x];
  
  /* Cave flags */
  info = cave_info[y][x];
  
  /* Hack -- rare random hallucination on non-outer walls */
  if (image && (feat < FEAT_PERM_SOLID) && (image_count-- <= 0))
    {
      int i;
      
      /* Display a random image, reset count. */
      image_count = randint(511);
      i = image_random();
      
      a = PICT_A(i);
      c = PICT_C(i);
    }
  
  /* Boring grids (floors, etc) */
  else if (feat <= FEAT_INVIS)
    {
      /* Memorized (or seen) floor */
      if ((info & (CAVE_MARK)) ||
	  (info & (CAVE_SEEN)))
	{
	  /* Get the floor feature */
	  f_ptr = &f_info[FEAT_FLOOR];
	  
	  /* Normal attr */
	  a = f_ptr->d_attr;
	  
	  /* Normal char */
	  c = f_ptr->d_char;
	  
	  /* Special lighting effects */
	  if (view_special_lite && (a == TERM_WHITE))
	    {
	      /* Handle "seen" grids */
	      if (info & (CAVE_SEEN))
		{
		  /* Only lit by "torch" lite */
		  if (view_yellow_lite && !(info & (CAVE_GLOW)) && 
		      (!is_daylight))
		    {
		      if (o_ptr->k_idx == 733)
			{
			  /* Hack for Lamp of Gwindor */
			  a = TERM_L_BLUE;
			}
		      else if ((check_ability(SP_UNLIGHT) || p_ptr->darkness) 
			       && (p_ptr->cur_lite <= 0))
			{
			  /* "Dark radius" */
			  a = TERM_L_DARK;
			}
		      else
			{
			  /* Use "yellow" */
			  a = TERM_YELLOW;
			}
		    }
		}
	      
	      /* Handle "blind" */
	      else if (p_ptr->blind)
		{
		  /* Use "dark gray" */
		  a = TERM_L_DARK;
		}
	      
	      /* Handle "dark" grids */
	      else if (!(info & (CAVE_GLOW)))
		{
		  /* Use "dark gray" */
		  a = TERM_L_DARK;
		}
	      
	      /* Handle "view_bright_lite" */
	      else if (view_bright_lite)
		{
		  /* Use "gray" */
		  a = TERM_SLATE;
		}
	    }
	      }
      
      /* Unknown */
      else
	{
	  /* Get the darkness feature */
	  f_ptr = &f_info[FEAT_NONE];
	  
	  /* Normal attr */
	  a = f_ptr->d_attr;
	  
	  /* Normal char */
	  c = f_ptr->d_char;
	}
    }
  
  /* Special terrain (trees etc) */
  else if ((feat >= FEAT_LAVA) && (feat <= FEAT_WEB))
    {
      /* Memorized (or seen) floor */
      if ((info & (CAVE_MARK)) ||
	  (info & (CAVE_SEEN)))
	{
	  /* Get the floor feature */
	  f_ptr = &f_info[feat];
	  
	  /* Normal attr */
	  a = f_ptr->d_attr;
	  
	  /* Normal char */
	  c = f_ptr->d_char;
	  
	  /* Special lighting effects */
	  if (view_special_lite)
	    {
	      /* Handle "seen" grids */
	      if (info & (CAVE_SEEN))
		{
		  /* Only lit by "torch" lite */
		  if (view_yellow_lite && !(info & (CAVE_GLOW))
		      && (a == TERM_WHITE) && (!is_daylight))
		    {
		      if (o_ptr->k_idx == 733)
			{
			  /* Hack for Lamp of Gwindor */
			  a = TERM_L_BLUE;
			}
		      else if ((check_ability(SP_UNLIGHT) || p_ptr->darkness) 
			       && (p_ptr->cur_lite <= 0))
			{
			  /* "Dark radius" */
			  a = TERM_L_DARK;
			}
		      else
			{
			  /* Use "yellow" */
			  a = TERM_YELLOW;
			}
		    }
		}
	      
	      /* Handle "blind" */
	      else if (p_ptr->blind)
		{
		  /* Use "dark gray" */
		  a = TERM_L_DARK;
		}
	      
	      /* Handle "dark" grids */
	      else if (!(info & (CAVE_GLOW)))
		{
		  /* Use "dark gray" */
		  a = TERM_L_DARK;
		}
	      
	      /* Handle "view_bright_lite" */
	      else if (view_bright_lite)
		{
		  /* Use "gray" */
		  a = TERM_SLATE;
		}
	    }
	}
      
      /* Unknown */
      else
	{
	  /* Get the darkness feature */
	  f_ptr = &f_info[FEAT_NONE];
	  
	  /* Normal attr */
	  a = f_ptr->d_attr;
	  
	  /* Normal char */
	  c = f_ptr->d_char;
	}
    }
  
  /* Interesting grids (non-floors) */
  else
    {
      /* Memorized grids */
      if (info & (CAVE_MARK))
	{
	  /* Apply "mimic" field */
	  feat = f_info[feat].mimic;
	  
	  /* Get the feature */
	  f_ptr = &f_info[feat];
	  
	  /* Normal attr */
	  a = f_ptr->d_attr;
	  
	  /* Normal char */
	  c = f_ptr->d_char;
	  
	  /* Special lighting effects (walls only) */
	  if (view_granite_lite && feat_supports_lighting(feat))
	    {
	      /* Handle "seen" grids */
	      if (info & (CAVE_SEEN))
		{
		  /* Use "white" */
		}
	      
	      /* Handle "blind" */
	      else if (p_ptr->blind)
		{
		  /* Use "dark gray" */
		  a = TERM_L_DARK;
		}
	      
	      /* Handle "view_bright_lite" */
	      else if (view_bright_lite)
		{
		  /* Use "gray" */
		  a = TERM_SLATE;
		}
	      else
		{
		  /* Use "white" */
		}
	    }
	}
      
      /* Unknown */
      else
	{
	  /* Get the darkness feature */
	  f_ptr = &f_info[FEAT_NONE];
	  
	  /* Normal attr */
	  a = f_ptr->d_attr;
	  
	  /* Normal char */
	  c = f_ptr->d_char;
	}
    }
  
  /* Objects */
  for (this_o_idx = cave_o_idx[y][x]; this_o_idx; this_o_idx = next_o_idx)
    {
      object_type *o_ptr;
      
      /* Hack to not display objects in trees */
      if ((cave_feat[y][x] == FEAT_TREE) || (cave_feat[y][x] == FEAT_TREE2) || 
	  (cave_feat[y][x] == FEAT_RUBBLE))
	break;
	      
      /* Get the object */
      o_ptr = &o_list[this_o_idx];
      
      /* Get the next object */
      next_o_idx = o_ptr->next_o_idx;
      
      /* Memorized objects */
      if (o_ptr->marked)
	{
	  /* Hack -- object hallucination */
	  if (image)
	    {
	      int i = image_object();
	      
	      a = PICT_A(i);
	      c = PICT_C(i);
	      
	      break;
	    }
	  
	  /* Normal attr */
	  a = object_attr(o_ptr);
	  
	  /* Normal char */
	  c = object_char(o_ptr);
	  
	  /* First marked object */
	  if (!show_piles) break;
	  
	  /* Special stack symbol */
	  if (++floor_num > 1)
	    {
	      object_kind *k_ptr;
	      
	      /* Get the "pile" feature */
	      k_ptr = &k_info[0];
	      
	      /* Normal attr */
	      a = k_ptr->d_attr;
	      
	      /* Normal char */
	      c = k_ptr->d_char;
	      
	      break;
	    }
	}
    }
  
  
  /* Monsters */
  if (m_idx > 0)
    {
      monster_type *m_ptr = &m_list[m_idx];
      
      /* Visible monster */
      if (m_ptr->ml)
	{
	  monster_race *r_ptr = &r_info[m_ptr->r_idx];
	  
	  byte da;
	  char dc;
	  
	  /* Desired attr */
	  da = r_ptr->d_attr;
	  
	  /* Desired char */
	  dc = r_ptr->d_char;
	  
	  /* Hack -- monster hallucination */
	  if (image)
	    {
	      int i = image_monster();
	      
	      a = PICT_A(i);
	      c = PICT_C(i);
	    }
	  
	  /* Ignore weird codes */
	  else if (avoid_other)
	    {
	      /* Use attr */
	      a = da;
	      
	      /* Use char */
	      c = dc;
	    }
	  
	  /* Special attr/char codes */
	  else if ((da & 0x80) && (dc & 0x80))
	    {
	      /* Use attr */
	      a = da;
	      
	      /* Use char */
	      c = dc;
	    }
	  
	  /* Multi-hued monster */
	  else if (r_ptr->flags1 & (RF1_ATTR_MULTI))
	    {
	      /* Multi-hued attr */
	      a = multi_hued_attr(r_ptr);
	      
	      /* Normal char */
	      c = dc;
	    }
	  
	  /* Normal monster (not "clear" in any way) */
	  else if (!(r_ptr->flags1 & (RF1_ATTR_CLEAR | RF1_CHAR_CLEAR)))
	    {
	      /* Use attr */
	      a = da;
	      
	      /* Use char */
	      c = dc;
	    }
	  
	  /* Hack -- Bizarre grid under monster */
	  else if ((a & 0x80) || (c & 0x80))
	    {
	      /* Use attr */
	      a = da;
	      
	      /* Use char */
	      c = dc;
	    }
	  
	  /* Normal char, Clear attr, monster */
	  else if (!(r_ptr->flags1 & (RF1_CHAR_CLEAR)))
	    {
	      /* Normal char */
	      c = dc;
	    }
	  
	  /* Normal attr, Clear char, monster */
	  else if (!(r_ptr->flags1 & (RF1_ATTR_CLEAR)))
	    {
	      /* Normal attr */
	      a = da;
	    }
	}
    }
  
  /* Handle "player" */
  else if ((m_idx < 0) && !(p_ptr->running && hidden_player))
    {
      monster_race *r_ptr = &r_info[0];
      
      /* Get the "player" attr */
      /*  DSV:  I've chosen the following sequence of colors to indicate
	  the player's current HP.  There are colors are left over, but I
	  left them in this comment for easy reference, in the likely case
	  that I decide to change the order of color changes.
	  
	  TERM_WHITE		90-100% of HP remaining
	  TERM_YELLOW		70- 89% of HP remaining
	  TERM_ORANGE		50- 69% of HP remaining
	  TERM_L_RED		30- 49% of HP remaining
	  TERM_RED		 0- 29% of HP remaining
	  
	  
	  TERM_SLATE		_% of HP remaining
	  TERM_UMBER		_% of HP remaining
	  TERM_L_UMBER	_% of HP remaining
	  TERM_BLUE		-% of HP remaining
	  TERM_L_BLUE		-% of HP remaining
	  TERM_GREEN		-% of HP remaining
	  TERM_L_GREEN	-% of HP remaining
	  TERM_DARK		-% of HP remaining
	  TERM_L_DARK		-% of HP remaining
	  TERM_L_WHITE	-% of HP remaining
	  TERM_VIOLET		-% of HP remaining
      */
      
      if (hp_changes_colour)
	{
	  switch(p_ptr->chp * 10 / p_ptr->mhp)
	    {
	    case 10:
	    case  9:	a = TERM_WHITE  ;	break;
	    case  8:
	    case  7:	a = TERM_YELLOW ;	break;
	    case  6:
	    case  5:	a = TERM_ORANGE ;	break;
	    case  4:
	    case  3:	a = TERM_L_RED  ;	break;
	    case  2:
	    case  1:
	    case  0:	a = TERM_RED    ;	break;
	    default:	a = TERM_WHITE  ;	break;
	    }

	  /*	  if ((a == TERM_WHITE) && (check_ability(SP_UNLIGHT)) &&
	      (p_ptr->cur_lite <= 0))
	      a = TERM_L_DARK; */
	}
      
      else a = r_ptr->d_attr;
      
      /* Get the "player" char */
      c = r_ptr->d_char;
    }
  
#ifdef MAP_INFO_MULTIPLE_PLAYERS
  /* Players */
  else if (m_idx < 0)
#else /* MAP_INFO_MULTIPLE_PLAYERS */
    /* Handle "player" */
    else if ((m_idx < 0) && !(p_ptr->running && hidden_player))
#endif /* MAP_INFO_MULTIPLE_PLAYERS */
      {
	monster_race *r_ptr = &r_info[0];
	
	/* Get the "player" attr */
	a = r_ptr->d_attr;
	
	/* Get the "player" char */
	c = r_ptr->d_char;
      }
  
  /* Result */
  (*ap) = a;
  (*cp) = c;
}

/*
 * Move the cursor to a given map location.
 *
 */
void move_cursor_relative(int y, int x)
{
  unsigned ky, kx;
  unsigned vy, vx;
  
  /* Location relative to panel */
  ky = (unsigned)(y - panel_row_min);
  
  /* Verify location */
  if (ky >= (unsigned)(SCREEN_HGT)) return;
  
  /* Location relative to panel */
  kx = (unsigned)(x - panel_col_min);
  
  /* Verify location */
  if (kx >= (unsigned)(SCREEN_WID)) return;
  
  /* Location in window */
  vy = ky + ROW_MAP;
  
  /* Location in window */
  vx = kx + COL_MAP;
  
  if (use_trptile)
    {
      vx += (use_bigtile ? 5 : 2) * kx;
      vy += 2 * ky;
    }
  else if (use_dbltile)
    {
      vx += (use_bigtile ? 3 : 1) * kx;
      vy += ky;
    }
  else if (use_bigtile) vx += kx;
  
  /* Go there */
  Term_gotoxy(vx, vy);
}


void big_queue_char(int x, int y, byte a, char c, byte a1, char c1)
{
  term *t = angband_term[0];

  /* Avoid warning */
  (void)c;

  /* Paranoia */
  if (use_bigtile || use_dbltile || use_trptile)
    {
      /* Mega-Hack : Queue dummy char */
      if (a & 0x80)
	Term_queue_char(t, x + 1, y, 255, -1, 0, 0);
      else
	Term_queue_char(t, x + 1, y, TERM_WHITE, ' ', a1, c1);
      
      /* Mega-Hack : Queue more dummy chars */
      if (use_dbltile || use_trptile)
	{
	  if (a & 0x80)
	    {
	      if (use_bigtile || use_trptile) 
		Term_queue_char(t, x + 2, y, 255, -1, 0, 0);
	      if (use_bigtile) 
		Term_queue_char(t, x + 3, y, 255, -1, 0, 0);
	      if (use_bigtile && use_trptile)
		{
		  Term_queue_char(t, x + 4, y, 255, -1, 0, 0);
		  Term_queue_char(t, x + 5, y, 255, -1, 0, 0);
		}
	      
	      Term_queue_char(t, x , y + 1, 255, -1, 0, 0);
	      Term_queue_char(t, x + 1, y + 1, 255, -1, 0, 0);
	      
	      if (use_bigtile || use_trptile) 
		Term_queue_char(t, x + 2, y + 1, 255, -1, 0, 0);
	      if (use_bigtile) 
		Term_queue_char(t, x + 3, y + 1, 255, -1, 0, 0);
	      if (use_bigtile && use_trptile)
		{
		  Term_queue_char(t, x + 4, y + 1, 255, -1, 0, 0);
		  Term_queue_char(t, x + 5, y + 1, 255, -1, 0, 0);
		}
	      
	      if (use_trptile)
		{
		  Term_queue_char(t, x , y + 2, 255, -1, 0, 0);
		  Term_queue_char(t, x + 1, y + 2, 255, -1, 0, 0);
		  Term_queue_char(t, x + 2, y + 2, 255, -1, 0, 0);
		  
		  if (use_bigtile)
		    {
		      Term_queue_char(t, x + 3, y + 2, 255, -1, 0, 0);
		      Term_queue_char(t, x + 4, y + 2, 255, -1, 0, 0);
		      Term_queue_char(t, x + 5, y + 2, 255, -1, 0, 0);
		    }
		}
	    }
	  else
	    {
	      if (use_bigtile || use_trptile) 
		Term_queue_char(t, x + 2, y, TERM_WHITE, ' ', a1, c1);
	      if (use_bigtile) 
		Term_queue_char(t, x + 3, y, TERM_WHITE, ' ', a1, c1);
	      if (use_bigtile && use_trptile)
		{
		  Term_queue_char(t, x + 4, y, TERM_WHITE, ' ', a1, c1);
		  Term_queue_char(t, x + 5, y, TERM_WHITE, ' ', a1, c1);
		}
	      
	      Term_queue_char(t, x , y + 1, TERM_WHITE, ' ', a1, c1);
	      Term_queue_char(t, x + 1, y + 1, TERM_WHITE, ' ', a1, c1);
	      
	      if (use_bigtile || use_trptile) 
		Term_queue_char(t, x + 2, y + 1, TERM_WHITE, ' ', a1, c1);
	      if (use_bigtile) 
		Term_queue_char(t, x + 3, y + 1, TERM_WHITE, ' ', a1, c1);
	      if (use_bigtile && use_trptile)
		{
		  Term_queue_char(t, x + 4, y + 1, TERM_WHITE, ' ', a1, c1);
		  Term_queue_char(t, x + 5, y + 1, TERM_WHITE, ' ', a1, c1);
		}
	      
	      if (use_trptile)
		{
		  Term_queue_char(t, x , y + 2, TERM_WHITE, ' ', a1, c1);
		  Term_queue_char(t, x + 1, y + 2, TERM_WHITE, ' ', a1, c1);
		  Term_queue_char(t, x + 2, y + 2, TERM_WHITE, ' ', a1, c1);
		  
		  if (use_bigtile)
		    {
		      Term_queue_char(t, x + 3, y + 2, TERM_WHITE, ' ', a1, c1);
		      Term_queue_char(t, x + 4, y + 2, TERM_WHITE, ' ', a1, c1);
		      Term_queue_char(t, x + 5, y + 2, TERM_WHITE, ' ', a1, c1);
		    }
		}
	    }
	}
    }
}

void big_putch(int x, int y, byte a, char c)
{
  /* Avoid warning */
  (void)c;
  
  /* Paranoia */
  if (use_bigtile || use_dbltile || use_trptile)
    {
      /* Mega-Hack : Queue dummy char */
      if (a & 0x80)
	Term_putch(x + 1, y, 255, -1);
      else
	Term_putch(x + 1, y, TERM_WHITE, ' ');
      
      /* Mega-Hack : Queue more dummy chars */
      if (use_dbltile || use_trptile)
	{
	  if (a & 0x80)
	    {
	      if (use_bigtile || use_trptile) 
		Term_putch(x + 2, y, 255, -1);
	      if (use_bigtile) 
		Term_putch(x + 3, y, 255, -1);
	      if (use_bigtile && use_trptile)
		{
		  Term_putch(x + 4, y, 255, -1);
		  Term_putch(x + 5, y, 255, -1);
		}

	      Term_putch(x , y + 1, 255, -1);
	      Term_putch(x + 1, y + 1, 255, -1);
	      
	      if (use_bigtile || use_trptile) 
		Term_putch(x + 2, y + 1, 255, -1);
	      if (use_bigtile) 
		Term_putch(x + 3, y + 1, 255, -1);
	      if (use_bigtile && use_trptile)
		{
		  Term_putch(x + 4, y + 1, 255, -1);
		  Term_putch(x + 5, y + 1, 255, -1);
		}
	      
	      if (use_trptile)
		{
		  Term_putch(x , y + 2, 255, -1);
		  Term_putch(x + 1, y + 2, 255, -1);
		  Term_putch(x + 2, y + 2, 255, -1);
		  
		  if (use_bigtile)
		    {
		      Term_putch(x + 3, y + 2, 255, -1);
		      Term_putch(x + 4, y + 2, 255, -1);
		      Term_putch(x + 5, y + 2, 255, -1);
		    }
		}
	    }
	  else
	    {
	      if (use_bigtile || use_trptile) 
		Term_putch(x + 2, y, TERM_WHITE, ' ');
	      if (use_bigtile) 
		Term_putch(x + 3, y, TERM_WHITE, ' ');
	      if (use_bigtile && use_trptile)
		{
		  Term_putch(x + 4, y, TERM_WHITE, ' ');
		  Term_putch(x + 5, y, TERM_WHITE, ' ');
		}
	      
	      Term_putch(x , y + 1, TERM_WHITE, ' ');
	      Term_putch(x + 1, y + 1, TERM_WHITE, ' ');
	      
	      if (use_bigtile || use_trptile) 
		Term_putch(x + 2, y + 1, TERM_WHITE, ' ');
	      if (use_bigtile) 
		Term_putch(x + 3, y + 1, TERM_WHITE, ' ');
	      if (use_bigtile && use_trptile)
		{
		  Term_putch(x + 4, y + 1, TERM_WHITE, ' ');
		  Term_putch(x + 5, y + 1, TERM_WHITE, ' ');
		}
	      
	      if (use_trptile)
		{
		  Term_putch(x , y + 2, TERM_WHITE, ' ');
		  Term_putch(x + 1, y + 2, TERM_WHITE, ' ');
		  Term_putch(x + 2, y + 2, TERM_WHITE, ' ');
		  
		  if (use_bigtile)
		    {
		      Term_putch(x + 3, y + 2, TERM_WHITE, ' ');
		      Term_putch(x + 4, y + 2, TERM_WHITE, ' ');
		      Term_putch(x + 5, y + 2, TERM_WHITE, ' ');
		    }
		}
	    }
	}
    }
}


/*
 * Display an attr/char pair at the given map location
 *
 * Note the inline use of "panel_contains()" for efficiency.
 *
 * Note the use of "Term_queue_char()" for efficiency.
 *
 * The main screen will always be at least 24x80 in size.
 */
void print_rel(char c, byte a, int y, int x)
{
  term *t = angband_term[0];

  unsigned ky, kx;
  unsigned vy, vx;

  /* Location relative to panel */
  ky = (unsigned)(y - panel_row_min);

  /* Verify location */
  if (ky >= (unsigned)(SCREEN_HGT)) return;
  
  /* Location relative to panel */
  kx = (unsigned)(x - panel_col_min);

  /* Verify location */
  if (kx >= (unsigned)(SCREEN_WID)) return;
  
  /* Location in window */
  vy = ky + ROW_MAP;

  /* Location in window */
  vx = kx + COL_MAP;

  if (use_trptile)
    {
      vx += (use_bigtile ? 5 : 2) * kx;
      vy += 2 * ky;
    }
  else if (use_dbltile)
    {
      vx += (use_bigtile ? 3 : 1) * kx;
      vy += ky;
    }
  else if (use_bigtile) vx += kx;
  
  /* Hack -- Queue it */
  Term_queue_char(t, vx, vy, a, c, 0, 0);
  
  if (use_bigtile || use_dbltile || use_trptile)
    {
      /* Mega-Hack : Queue dummy char */
      big_queue_char(vx, vy, a, c, 0, 0);
    }
  
  return;
}

/*
 * Memorize interesting viewable object/features in the given grid
 *
 * This function should only be called on "legal" grids.
 *
 * This function will memorize the object and/or feature in the given grid,
 * if they are (1) see-able and (2) interesting.  Note that all objects are
 * interesting, all terrain features except floors (and invisible traps) are
 * interesting, and floors (and invisible traps) are interesting sometimes
 * (depending on various options involving the illumination of floor grids).
 *
 * The automatic memorization of all objects and non-floor terrain features
 * as soon as they are displayed allows incredible amounts of optimization
 * in various places, especially "map_info()" and this function itself.
 *
 * Note that the memorization of objects is completely separate from the
 * memorization of terrain features, preventing annoying floor memorization
 * when a detected object is picked up from a dark floor, and object
 * memorization when an object is dropped into a floor grid which is
 * memorized but out-of-sight.
 *
 * This function should be called every time the "memorization" of a grid
 * (or the object in a grid) is called into question, such as when an object
 * is created in a grid, when a terrain feature "changes" from "floor" to
 * "non-floor", and when any grid becomes "see-able" for any reason.
 *
 * This function is called primarily from the "update_view()" function, for
 * each grid which becomes newly "see-able".
 */
void note_spot(int y, int x)
{
  byte info;
  
  s16b this_o_idx, next_o_idx = 0;
  
  
  /* Get cave info */
  info = cave_info[y][x];
  
  /* Require "seen" flag */
  if (!(info & (CAVE_SEEN))) return;
  
  
  /* Hack -- memorize objects */
  for (this_o_idx = cave_o_idx[y][x]; this_o_idx; this_o_idx = next_o_idx)
    {
      object_type *o_ptr = &o_list[this_o_idx];
      
      /* Acquire next object */
      next_o_idx = o_ptr->next_o_idx;
      
      /* Memorize objects */
      o_ptr->marked = TRUE;
    }
  
  
  /* Hack -- memorize grids */
  if (!(info & (CAVE_MARK)))
    {
      /* Memorize some "boring" grids */
      if (cave_feat[y][x] <= FEAT_INVIS)
	{
	  /* Option -- memorize certain floors */
	  if (((info & (CAVE_GLOW)) && view_perma_grids) ||
	      view_torch_grids)
	    {
	      /* Memorize */
	      cave_info[y][x] |= (CAVE_MARK);
	    }
	}
      
      /* Memorize all "interesting" grids */
      else
	{
	  /* Memorize */
	  cave_info[y][x] |= (CAVE_MARK);
	}
    }
}


/*
 * Redraw (on the screen) a given MAP location
 *
 * This function should only be called on "legal" grids
 */
void lite_spot(int y, int x)
{
  /* Redraw if on screen */
  if (panel_contains(y, x) && in_bounds(y, x))
    {
      term *t = angband_term[0];

      byte a;
      char c;
      
      unsigned ky, kx;
      unsigned vy, vx;

      byte ta;
      char tc;
      
      /* Location relative to panel */
      ky = (unsigned)(y - panel_row_min);
      
      /* Verify location */
      if (ky >= (unsigned)(SCREEN_HGT)) return;

      /* Location relative to panel */
      kx = (unsigned)(x - panel_col_min);

      /* Verify location */
      if (kx >= (unsigned)(SCREEN_WID)) return;

      /* Location in window */
      vy = ky + ROW_MAP;

      /* Location in window */
      vx = kx + COL_MAP;

      if (use_trptile)
	{
	  vx += (use_bigtile ? 5 : 2) * kx;
	  vy += 2 * ky;
	}
      else if (use_dbltile)
	{
	  vx += (use_bigtile ? 3 : 1) * kx;
	  vy += ky;
	}
      else if (use_bigtile) vx += kx;
      
      /* Examine the grid */
      map_info(y, x, &a, &c, &ta, &tc);
      
      /* Hack -- Queue it */
      Term_queue_char(t, vx, vy, a, c, ta, tc);

      if (use_bigtile || use_dbltile || use_trptile)
	{
	  big_queue_char(vx, vy, a, c, TERM_WHITE, ' ');
	}
    }
}



void prt_map(void)
{
  term *t = angband_term[0];

  byte a;
  char c;
  byte ta;
  char tc;
  
  int y, x;
  int vy, vx;
  int ty, tx;
  
  /* Assume screen */
  ty = panel_row_min + SCREEN_HGT;
  tx = panel_col_min + SCREEN_WID;

  /* Dump the map */
  for (y = panel_row_min, vy = ROW_MAP; y < ty; vy++, y++)
    {
      for (x = panel_col_min, vx = COL_MAP; x < tx; vx++, x++)
	{
	  /* Check bounds */
	  if (!in_bounds(y, x)) continue;
	  
	  /* Determine what is there */
	  map_info(y, x, &a, &c, &ta, &tc);
	  
	  /* Hack -- Queue it */
	  Term_queue_char(t, vx, vy, a, c, ta, tc);
	  
	  if (use_bigtile || use_dbltile || use_trptile)
	    {
	      big_queue_char(vx, vy, a, c, TERM_WHITE, ' ');
	      
	      if (use_trptile)
		{
		  vx += (use_bigtile ? 5 : 2);
		}
	      else
		vx+= ((use_dbltile && use_bigtile) ? 3 : 1);
	    }
	}
      
      if (use_trptile)
	vy++;
      
      if (use_dbltile || use_trptile)
	vy++;
    }
}



/*
 * Display highest priority object in the RATIO by RATIO area
 */
#define RATIO 3

/*
 * Display the entire map
 */
#define MAP_HGT (DUNGEON_HGT / RATIO)
#define MAP_WID (DUNGEON_WID / RATIO)

/*
 * Hack -- priority array (see below)
 *
 * Note that all "walls" always look like "secret doors" (see "map_info()").
 */
static byte priority_table[][2] =
{
  /* Dark */
  { FEAT_NONE, 2 },
  
  /* Floors, grass */
  { FEAT_FLOOR, 5 },
  { FEAT_GRASS, 5 },
  
  /* Water. */
  { FEAT_WATER, 6 },

  /* Shop walls */
  { FEAT_PERM_EXTRA, 7},
  
  /* Walls */
  { FEAT_SECRET, 10 },
  
  /* Quartz */
  { FEAT_QUARTZ, 11 },
  
  /* Magma */
  { FEAT_MAGMA, 12 },
  
  /* Rubble */
  { FEAT_RUBBLE, 13 },
  
  /* Trees and lava. */
  { FEAT_LAVA, 14 },
  { FEAT_TREE, 14 },
  { FEAT_TREE2, 14 },
  
  /* Open doors */
  { FEAT_OPEN, 15 },
  { FEAT_BROKEN, 15 },
  
  /* Closed doors */
  { FEAT_DOOR_HEAD + 0x00, 17 },

  /* Shops */  
  { FEAT_SHOP_HEAD + 0x00, 17},
  { FEAT_SHOP_HEAD + 0x01, 17},
  { FEAT_SHOP_HEAD + 0x02, 17},
  { FEAT_SHOP_HEAD + 0x03, 17},
  { FEAT_SHOP_HEAD + 0x04, 17},
  { FEAT_SHOP_HEAD + 0x05, 17},
  { FEAT_SHOP_HEAD + 0x06, 17},
  { FEAT_SHOP_HEAD + 0x07, 17},
  { FEAT_SHOP_HEAD + 0x08, 17},
  { FEAT_SHOP_HEAD + 0x09, 17},

  /* Hidden gold */
  { FEAT_QUARTZ_K, 19 },
  { FEAT_MAGMA_K, 19 },
  
  /* Monster traps */
  { FEAT_MTRAP_HEAD, 20},
  { FEAT_MTRAP_TAIL, 20},
  { FEAT_MTRAP_BASE, 20},
  { FEAT_MTRAP_STURDY, 20},
  { FEAT_MTRAP_NET, 20},
  { FEAT_MTRAP_CONF, 20},
  { FEAT_MTRAP_POISON, 20},
  { FEAT_MTRAP_SPIRIT, 20},
  { FEAT_MTRAP_ELEC, 20},
  { FEAT_MTRAP_EXPLOSIVE, 20},
  { FEAT_MTRAP_PORTAL, 20},
  { FEAT_MTRAP_STASIS, 20},
  { FEAT_MTRAP_DRAIN_LIFE, 20},
  { FEAT_MTRAP_UNMAGIC, 20},
  { FEAT_MTRAP_DISPEL_M, 20},

  /* Stairs */
  { FEAT_LESS, 25 },
  { FEAT_MORE, 25 },
  { FEAT_LESS_NORTH, 25 },
  { FEAT_MORE_NORTH, 25 },
  { FEAT_LESS_EAST, 25 },
  { FEAT_MORE_EAST, 25 },
  { FEAT_LESS_SOUTH, 25 },
  { FEAT_MORE_SOUTH, 25 },
  { FEAT_LESS_WEST, 25 },
  { FEAT_MORE_WEST, 25 },
  
  /* End */
  { 0, 0 }
};


/*
 * Hack -- a priority function (see below)
 */
static byte priority(byte a, char c)
{
  int i, p0, p1;
  
  feature_type *f_ptr;
  
  /* Scan the table */
  for (i = 0; TRUE; i++)
    {
      /* Priority level */
      p1 = priority_table[i][1];
      
      /* End of table */
      if (!p1) break;
      
      /* Feature index */
      p0 = priority_table[i][0];
      
      /* Access the feature */
      f_ptr = &f_info[p0];

      /* Hack - floor more important in wilderness */
      if ((stage_map[p_ptr->stage][STAGE_TYPE] != CAVE) && 
	  (stage_map[p_ptr->stage][STAGE_TYPE] != TOWN) && (p0 == FEAT_FLOOR))
	p1 = 19;
      
      /* Check character and attribute, accept matches */
      if ((f_ptr->x_char == c) && (f_ptr->x_attr == a)) return (p1);
    }
  
  /* Default */
  return (20);
}


#if 0
/*
 * Tunnels are important.  (Whist bare floor is not.)
 */
static int priority_tunnel(int y, int x)
{
  int i, count = 0;
  int yy, xx;
  
  /* Count number of floors around square */
  for (i = 1; i < 10; i++)
    {
      yy=y+ddy[i];
      xx=x+ddx[i];
      
      if ((in_bounds(yy, xx)) && (cave_feat[yy][xx] < FEAT_MAGMA))
	count++;
    }
  
  /* Three or less floor squares - Important */
  if (count < 4) return (19);
  
  /* Not important. */
  return (0);
}
#endif

/*
 * Display a "small-scale" map of the dungeon in the active Term
 *
 * Note that the "map_info()" function must return fully colorized
 * data or this function will not work correctly.
 *
 * Note that this function must "disable" the special lighting
 * effects so that the "priority" function will work.
 *
 * Note the use of a specialized "priority" function to allow this
 * function to work with any graphic attr/char mappings, and the
 * attempts to optimize this function where possible.
 *
 * cx and cy are offsets from the position of the player.  This
 * allows the map to be shifted around - but only works in the
 * wilderness.  cx and cy return the position of the player on the
 * possibly shifted map.
 */
void display_map(int *cy, int *cx, bool small)
{
  int py = p_ptr->py;
  int px = p_ptr->px;

  int map_hgt, map_wid;
  int dungeon_hgt, dungeon_wid, top_row, left_col;
  int row, col;

  int x, y;
  
  byte ta;
  char tc;
  
  byte sa;
  char sc;


  byte tp;
  
  /* Large array on the stack */
  byte mp[DUNGEON_HGT][DUNGEON_WID];

  bool old_view_special_lite;
  bool old_view_granite_lite;
  bool old_bigtile = use_bigtile;
  bool old_dbltile = use_dbltile;
  bool old_trptile = use_trptile;

  monster_race *r_ptr = &r_info[0];

  /* Desired map height */
  map_hgt = Term->hgt - 2;
  map_wid = Term->wid - 2;
  
  /* Adjust for town */
  dungeon_hgt = (p_ptr->depth ? DUNGEON_HGT : 2 * DUNGEON_HGT / 3);
  dungeon_wid = (p_ptr->depth ? DUNGEON_WID : 2 * DUNGEON_WID / 3);
  if (!(p_ptr->depth) && (p_ptr->stage < 151)) dungeon_wid = DUNGEON_WID / 2;
  top_row = (p_ptr->depth ? 0 : DUNGEON_HGT / 3);
  left_col = (p_ptr->depth ? 0 : DUNGEON_WID / 3);
  
  /* Prevent accidents */
  if (map_hgt > dungeon_hgt) map_hgt = dungeon_hgt;
  if (map_wid > dungeon_wid) map_wid = dungeon_wid;
  
  /* Prevent accidents */
  if ((map_wid < 1) || (map_hgt < 1)) return;
  

  /* Save lighting effects */
  old_view_special_lite = view_special_lite;
  old_view_granite_lite = view_granite_lite;
  
  /* Disable lighting effects */
  view_special_lite = FALSE;
  view_granite_lite = FALSE;

  /* Disable tiles for subwindow */
  if (small) 
    {
      use_bigtile = FALSE;
      use_dbltile = FALSE;
      use_trptile = FALSE;
    }

  /* Nothing here */
  ta = TERM_WHITE;
  tc = ' ';
  
  /* Clear the priorities */
  for (y = 0; y < map_hgt; ++y)
    {
      for (x = 0; x < map_wid; ++x)
	{
	  /* No priority */
	  mp[y][x] = 0;
	}
    }
  
  /* Clear the screen (but don't force a redraw) */
  clear_from(0);
  
  /* Corners */
  x = map_wid + 1;
  y = map_hgt + 1;
  
  /* Draw the corners */
  Term_putch(0, 0, ta, '+');
  Term_putch(x, 0, ta, '+');
  Term_putch(0, y, ta, '+');
  Term_putch(x, y, ta, '+');
  
  /* Draw the horizontal edges */
  for (x = 1; x <= map_wid; x++)
    {
      Term_putch(x, 0, ta, '-');
      Term_putch(x, y, ta, '-');
    }
  
  /* Draw the vertical edges */
  for (y = 1; y <= map_hgt; y++)
    {
      Term_putch(0, y, ta, '|');
      Term_putch(x, y, ta, '|');
    }
  
  
  /* Analyze the actual map */
  for (y = top_row; y < dungeon_hgt; y++)
    {
      for (x = left_col; x < dungeon_wid; x++)
	{
	  row = ((y - top_row) * map_hgt / dungeon_hgt);
	  col = ((x - left_col) * map_wid / dungeon_wid);
	  
	  if (use_trptile)
	    {
	      col = col - (col % (use_bigtile ? 6 : 3));
	      row = row - (row % 3);
	    }
	  else if (use_dbltile)
	    {
	      col = col & ~(use_bigtile ? 3 : 1);
	      row = row & ~1;
	    }
	  else if (use_bigtile)
	    col = col & ~1;
	  
	  /* Get the attr/char at that map location */
	  if (small) map_info_default(y, x, &ta, &tc);
	  else map_info(y, x, &ta, &tc, &sa, &sc);

	  /* Get the priority of that attr/char */
	  tp = priority(ta, tc);
	  
	  /* Save "best" */
	  if (mp[row][col] < tp)
	    {
	      /* Add the character */
	      Term_putch(col + 1, row + 1, ta, tc);
	      
	      if (use_bigtile || use_dbltile || use_trptile)
		{
		  big_putch(col + 1, row + 1, ta, tc);
		}
	      
	      /* Save priority */
	      mp[row][col] = tp;
	    }
	}
    }
  
  
  /* Player location */
  row = ((py - top_row) * map_hgt / dungeon_hgt);
  col = ((px - left_col) * map_wid / dungeon_wid);
  
  if (use_trptile)
    {
      col = col - (col % (use_bigtile ? 6 : 3));
      row = row - (row % 3);
    }
  else if (use_dbltile)
    {
      col = col & ~(use_bigtile ? 3 : 1);
      row = row & ~1;
    }
  else if (use_bigtile)
    col = col & ~1;
  
  /*** Make sure the player is visible ***/
  
  /* Get the "player" attr */
  ta = (small ? r_ptr->d_attr : r_ptr->x_attr);
  
  /* Get the "player" char */
  tc = (small ? r_ptr->d_char : r_ptr->x_char);
  
  /* Draw the player */
  Term_putch(col + 1, row + 1, ta, tc);
  
  if (use_bigtile || use_dbltile || use_trptile)
    {
      big_putch(col + 1, row + 1, ta, tc);
    }
  
  /* Return player location */
  if (cy != NULL) (*cy) = row + 1;
  if (cx != NULL) (*cx) = col + 1;
  
  
  /* Restore tiles for subwindow */
  if (small) 
    {
      use_bigtile = old_bigtile;
      use_dbltile = old_dbltile;
      use_trptile = old_trptile;
    }

  /* Restore lighting effects */
  view_special_lite = old_view_special_lite;
  view_granite_lite = old_view_granite_lite;
}

/*
 * Display a map of the type of wilderness surrounding the current stage
 */
void regional_map(int num, int size)
{
  int i, j, col, row;
  int *stage = malloc( size * sizeof (*stage) );
  int north, east, south, west;
  cptr lev;

  /* Get the side length */
  num = 2 * num + 1;

  /* Initialise */
  for (i = 0; i < size; i++)
    stage[i] = 0;
    
  /* Set the centre */
  stage[size/2] = p_ptr->stage;

  /* Redo the right number of times */
  for (j = 0; j < num; j++)
    {
      /* Pass across the whole array */
      for (i = 0; i < size; i++)
	{
	  /* See what's adjacent */
	  north = (i > (num - 1) ? (i - num) : -1);
	  east = ((i % num) != (num - 1) ? (i + 1): -1);
	  south = (i < (size - num) ? (i + num) : -1);
	  west = ((i % num) ? (i - 1) : -1);

	  /* Set them */
	  if ((north >= 0) && (stage_map[stage[i]][NORTH]) && (!stage[north]))
	    stage[north] = stage_map[stage[i]][NORTH];
	  if ((east >= 0) && (stage_map[stage[i]][EAST]) && (!stage[east]))
	    stage[east] = stage_map[stage[i]][EAST];
	  if ((south >= 0) && (stage_map[stage[i]][SOUTH]) && (!stage[south]))
	    stage[south] = stage_map[stage[i]][SOUTH];
	  if ((west >= 0) && (stage_map[stage[i]][WEST]) && (!stage[west]))
	    stage[west] = stage_map[stage[i]][WEST];
	}
    }

  /* Now print the info */
  for (i = 0; i < size; i++)
    {
      /* Nowhere */
      if (!stage[i]) continue;

      /* Get the place */
      col = (i % num) * 10 + COL_MAP;
      row = (i / num) * 4 + ROW_MAP;
      
      /* Get the level string */
      lev = format("Level %d", stage_map[stage[i]][DEPTH]);
      
      switch (stage_map[stage[i]][STAGE_TYPE])
	    {
	    case TOWN:
	      {
		c_put_str(TERM_SLATE, "Town", row, col);
		break;
	      }
	    case PLAIN:
	      {
		c_put_str(TERM_UMBER, "Plain", row, col);
		break;
	      }
	    case FOREST:
	      {
		c_put_str(TERM_GREEN, "Forest", row, col);
		break;
	      }
	    case MOUNTAIN:
	      {
		c_put_str(TERM_L_DARK, "Mountain", row, col);
		break;
	      }
	    case SWAMP:
	      {
		c_put_str(TERM_L_GREEN, "Swamp", row, col);
		break;
	      }
	    case RIVER:
	      {
		c_put_str(TERM_BLUE, "River", row, col);
		break;
	      }
	    case DESERT:
	      {
		c_put_str(TERM_L_UMBER, "Desert", row, col);
		break;
	      }
	    case VALLEY:
	      {
		c_put_str(TERM_RED, "Valley", row, col);
		break;
	      }
	    default:
	      break;
	    }
      if (stage[i] == p_ptr->stage)
	c_put_str(TERM_VIOLET, "Current ", row, col);
      c_put_str(TERM_WHITE, lev, row + 1, col);
      if (stage_map[stage[i]][EAST]) 
	c_put_str(TERM_WHITE, "-", row + 1, col + 8);
      if (stage_map[stage[i]][DOWN]) 
	{
	  if (stage_map[stage[i]][STAGE_TYPE] == MOUNTAINTOP)
	    switch (stage_map[stage_map[stage[i]][DOWN]][STAGE_TYPE])
	      {
	      case TOWN:
		{
		  c_put_str(TERM_SLATE, "(Town)", row + 2, col);
		  break;
		}
	      case PLAIN:
		{
		  c_put_str(TERM_UMBER, "(Plain)", row + 2, col);
		  break;
		}
	      case FOREST:
		{
		  c_put_str(TERM_GREEN, "(Forest)", row + 2, col);
		  break;
		}
	      case MOUNTAIN:
		{
		  c_put_str(TERM_L_DARK, "(Mountain)", row + 2, col);
		  break;
		}
	      case RIVER:
		{
		  c_put_str(TERM_BLUE, "(River)", row + 2, col);
		  break;
		}
	      case DESERT:
		{
		  c_put_str(TERM_L_UMBER, "(Desert)", row + 2, col);
		  break;
		}
	      case VALLEY:
		{
		  c_put_str(TERM_RED, "(Valley)", row + 2, col);
		  break;
		}
	      default:
		break;
	      }
	  else
	    c_put_str(TERM_L_RED, "(Dungeon)", row + 2, col);
	}
      if (stage_map[stage[i]][SOUTH]) 
	c_put_str(TERM_WHITE, "|", row + 3, col + 3);
      if ((stage_map[stage[i]][SOUTH]) && (!stage_map[stage[i]][DOWN])) 
	c_put_str(TERM_WHITE, "|", row + 2, col + 3);
    }
  free(stage);

}

/*
 * Display a "small-scale" map of the dungeon.
 *
 * Note that the "player" is always displayed on the map.
 */
void do_cmd_view_map(void)
{
  int cy, cx;
  int wid, hgt, num_down, num_across, num;
  
  /* Get size */
  Term_get_size(&wid, &hgt);
  
  /* Get dimensions for the regional map */
  num_down = (hgt - 6)/8;
  num_across = (wid - 24)/20;
  num = (num_down < num_across ? num_down : num_across);

  /* Save screen */
  screen_save();
  
  /* Note */
  prt("Please wait...", 0, 0);
  
  /* Flush */
  Term_fresh();
  
  /* Clear the screen */
  Term_clear();
  
  /* Display the map */
  display_map(&cy, &cx, FALSE);
  
  /* Wait for it */
  put_str("Hit any key to continue", hgt - 1, (wid - COL_MAP) / 2);
  
  /* Hilite the player */
  move_cursor(cy, cx);
  
  /* Get any key */
  (void)inkey_ex();

  /* Regional map if not in the dungeon */
  if (stage_map[p_ptr->stage][STAGE_TYPE] != CAVE)
    {
  
      /* Flush */
      Term_fresh();
  
      /* Clear the screen */
      Term_clear();
  
      /* Display the regional map */
      regional_map(num, (2 * num + 1) * (2 * num + 1));
  
      /* Wait for it */
      put_str("Hit any key to continue", hgt - 1, (wid - COL_MAP) / 2);
  
      /* Get any key */
      (void)inkey_ex();
    }
  
  /* Load screen */
  screen_load();
}


/*
 * Some comments on the dungeon related data structures and functions...
 *
 * Angband is primarily a dungeon exploration game, and it should come as
 * no surprise that the internal representation of the dungeon has evolved
 * over time in much the same way as the game itself, to provide semantic
 * changes to the game itself, to make the code simpler to understand, and
 * to make the executable itself faster or more efficient in various ways.
 *
 * There are a variety of dungeon related data structures, and associated
 * functions, which store information about the dungeon, and provide methods
 * by which this information can be accessed or modified.
 *
 * Some of this information applies to the dungeon as a whole, such as the
 * list of unique monsters which are still alive.  Some of this information
 * only applies to the current dungeon level, such as the current depth, or
 * the list of monsters currently inhabiting the level.   And some of the
 * information only applies to a single grid of the current dungeon level,
 * such as whether the grid is illuminated, or whether the grid contains a
 * monster, or whether the grid can be seen by the player.  If Angband was
 * to be turned into a multi-player game, some of the information currently
 * associated with the dungeon should really be associated with the player,
 * such as whether a given grid is viewable by a given player.
 *
 * One of the major bottlenecks in ancient versions of Angband was in the
 * calculation of "line of sight" from the player to various grids, such
 * as those containing monsters, using the relatively expensive "los()"
 * function.  This was such a nasty bottleneck that a lot of silly things
 * were done to reduce the dependancy on "line of sight", for example, you
 * could not "see" any grids in a lit room until you actually entered the
 * room, at which point every grid in the room became "illuminated" and
 * all of the grids in the room were "memorized" forever.  Other major
 * bottlenecks involved the determination of whether a grid was lit by the
 * player's torch, and whether a grid blocked the player's line of sight.
 * These bottlenecks led to the development of special new functions to
 * optimize issues involved with "line of sight" and "torch lit grids".
 * These optimizations led to entirely new additions to the game, such as
 * the ability to display the player's entire field of view using different
 * colors than were used for the "memorized" portions of the dungeon, and
 * the ability to memorize dark floor grids, but to indicate by the way in
 * which they are displayed that they are not actually illuminated.  And
 * of course many of them simply made the game itself faster or more fun.
 * Also, over time, the definition of "line of sight" has been relaxed to
 * allow the player to see a wider "field of view", which is slightly more
 * realistic, and only slightly more expensive to maintain.
 *
 * Currently, a lot of the information about the dungeon is stored in ways
 * that make it very efficient to access or modify the information, while
 * still attempting to be relatively conservative about memory usage, even
 * if this means that some information is stored in multiple places, or in
 * ways which require the use of special code idioms.  For example, each
 * monster record in the monster array contains the location of the monster,
 * and each cave grid has an index into the monster array, or a zero if no
 * monster is in the grid.  This allows the monster code to efficiently see
 * where the monster is located, while allowing the dungeon code to quickly
 * determine not only if a monster is present in a given grid, but also to
 * find out which monster.  The extra space used to store the information
 * twice is inconsequential compared to the speed increase.
 *
 * Some of the information about the dungeon is used by functions which can
 * constitute the "critical efficiency path" of the game itself, and so the
 * way in which they are stored and accessed has been optimized in order to
 * optimize the game itself.  For example, the "update_view()" function was
 * originally created to speed up the game itself (when the player was not
 * running), but then it took on extra responsibility as the provider of the
 * new "special effects lighting code", and became one of the most important
 * bottlenecks when the player was running.  So many rounds of optimization
 * were performed on both the function itself, and the data structures which
 * it uses, resulting eventually in a function which not only made the game
 * faster than before, but which was responsible for even more calculations
 * (including the determination of which grids are "viewable" by the player,
 * which grids are illuminated by the player's torch, and which grids can be
 * "seen" in some way by the player), as well as for providing the guts of
 * the special effects lighting code, and for the efficient redisplay of any
 * grids whose visual representation may have changed.
 *
 * Several pieces of information about each cave grid are stored in various
 * two dimensional arrays, with one unit of information for each grid in the
 * dungeon.  Some of these arrays have been intentionally expanded by a small
 * factor to make the two dimensional array accesses faster by allowing the
 * use of shifting instead of multiplication.
 *
 * Several pieces of information about each cave grid are stored in the
 * "cave_info" array, which is a special two dimensional array of bytes,
 * one for each cave grid, each containing eight separate "flags" which
 * describe some property of the cave grid.  These flags can be checked and
 * modified extremely quickly, especially when special idioms are used to
 * force the compiler to keep a local register pointing to the base of the
 * array.  Special location offset macros can be used to minimize the number
 * of computations which must be performed at runtime.  Note that using a
 * byte for each flag set may be slightly more efficient than using a larger
 * unit, so if another flag (or two) is needed later, and it must be fast,
 * then the two existing flags which do not have to be fast should be moved
 * out into some other data structure and the new flags should take their
 * place.  This may require a few minor changes in the savefile code.
 *
 * The "CAVE_ROOM" flag is saved in the savefile and is used to determine
 * which grids are part of "rooms", and thus which grids are affected by
 * "illumination" spells.  This flag does not have to be very fast.
 *
 * The "CAVE_ICKY" flag is saved in the savefile and is used to determine
 * which grids are part of "vaults", and thus which grids cannot serve as
 * the destinations of player teleportation.  This flag does not have to
 * be very fast.
 *
 * The "CAVE_MARK" flag is saved in the savefile and is used to determine
 * which grids have been "memorized" by the player.  This flag is used by
 * the "map_info()" function to determine if a grid should be displayed.
 * This flag is used in a few other places to determine if the player can
 * "know" about a given grid.  This flag must be very fast. 
 *
 * The "CAVE_GLOW" flag is saved in the savefile and is used to determine
 * which grids are "permanently illuminated".  This flag is used by the
 * "update_view()" function to help determine which viewable flags may
 * be "seen" by the player.  This flag is used by the "map_info" function
 * to determine if a grid is only lit by the player's torch.  This flag
 * has special semantics for wall grids (see "update_view()").  This flag
 * must be very fast.
 *
 * The "CAVE_WALL" flag is used to determine which grids block the player's
 * line of sight.  This flag is used by the "update_view()" function to
 * determine which grids block line of sight, and to help determine which
 * grids can be "seen" by the player.  This flag must be very fast.
 *
 * The "CAVE_VIEW" flag is used to determine which grids are currently in
 * line of sight of the player.   This flag is set by (and used by) the
 * "update_view()" function.  This flag is used by any code which needs to
 * know if the player can "view" a given grid.  This flag is used by the
 * "map_info()" function for some optional special lighting effects.  The
 * "player_has_los_bold()" macro wraps an abstraction around this flag, but
 * certain code idioms are much more efficient.   This flag is used to check
 * if a modification to a terrain feature might affect the player's field of
 * view.  This flag is used to see if certain monsters are "visible" to the
 * player.  This flag is used to allow any monster in the player's field of
 * view to "sense" the presence of the player.  This flag must be very fast.
 *
 * The "CAVE_SEEN" flag is used to determine which grids are currently in
 * line of sight of the player and also illuminated in some way.  This flag
 * is set by the "update_view()" function, using computations based on the
 * "CAVE_VIEW" and "CAVE_WALL" and "CAVE_GLOW" flags of various grids.  This
 * flag is used by any code which needs to know if the player can "see" a
 * given grid.  This flag is used by the "map_info()" function both to see
 * if a given "boring" grid can be seen by the player, and for some optional
 * special lighting effects.  The "player_can_see_bold()" macro wraps an
 * abstraction around this flag, but certain code idioms are much more
 * efficient.  This flag is used to see if certain monsters are "visible" to
 * the player.  This flag is never set for a grid unless "CAVE_VIEW" is also
 * set for the grid.  Whenever the "CAVE_WALL" or "CAVE_GLOW" flag changes
 * for a grid which has the "CAVE_VIEW" flag set, the "CAVE_SEEN" flag must
 * be recalculated.  The simplest way to do this is to call "forget_view()"
 * and "update_view()" whenever the "CAVE_WALL" or "CAVE_GLOW" flags change
 * for a grid which has "CAVE_VIEW" set.  This flag must be very fast.
 *
 * The "CAVE_TEMP" flag is used for a variety of temporary purposes.  This
 * flag is used to determine if the "CAVE_SEEN" flag for a grid has changed
 * during the "update_view()" function.   This flag is used to "spread" light
 * or darkness through a room.  This flag is used by the "monster flow code".
 * This flag must always be cleared by any code which sets it, often, this
 * can be optimized by the use of the special "temp_g", "temp_y", "temp_x"
 * arrays (and the special "temp_n" global).  This flag must be very fast.
 *
 * Note that the "CAVE_MARK" flag is used for many reasons, some of which
 * are strictly for optimization purposes.  The "CAVE_MARK" flag means that
 * even if the player cannot "see" the grid, he "knows" about the terrain in
 * that grid.  This is used to "memorize" grids when they are first "seen" by
 * the player, and to allow certain grids to be "detected" by certain magic.
 * Note that most grids are always memorized when they are first "seen", but
 * "boring" grids (floor grids) are only memorized if the "view_torch_grids"
 * option is set, or if the "view_perma_grids" option is set, and the grid
 * in question has the "CAVE_GLOW" flag set.
 *
 * Objects are "memorized" in a different way, using a special "marked" flag
 * on the object itself, which is set when an object is observed or detected.
 * This allows objects to be "memorized" independant of the terrain features.
 *
 * The "update_view()" function is an extremely important function.  It is
 * called only when the player moves, significant terrain changes, or the
 * player's blindness or torch radius changes.  Note that when the player
 * is resting, or performing any repeated actions (like digging, disarming,
 * farming, etc), there is no need to call the "update_view()" function, so
 * even if it was not very efficient, this would really only matter when the
 * player was "running" through the dungeon.  It sets the "CAVE_VIEW" flag
 * on every cave grid in the player's field of view, and maintains an array
 * of all such grids in the global "view_g" array.  It also checks the torch
 * radius of the player, and sets the "CAVE_SEEN" flag for every grid which
 * is in the "field of view" of the player and which is also "illuminated",
 * either by the players torch (if any) or by any permanent light source.
 * It could use and help maintain information about multiple light sources,
 * which would be helpful in a multi-player version of Angband.
 *
 * The "update_view()" function maintains the special "view_g" array, which
 * contains exactly those grids which have the "CAVE_VIEW" flag set.  This
 * array is used by "update_view()" to (only) memorize grids which become
 * newly "seen", and to (only) redraw grids whose "seen" value changes, which
 * allows the use of some interesting (and very efficient) "special lighting
 * effects".  In addition, this array could be used elsewhere to quickly scan
 * through all the grids which are in the player's field of view.
 *
 * Note that the "update_view()" function allows, among other things, a room
 * to be "partially" seen as the player approaches it, with a growing cone
 * of floor appearing as the player gets closer to the door.  Also, by not
 * turning on the "memorize perma-lit grids" option, the player will only
 * "see" those floor grids which are actually in line of sight.   And best
 * of all, you can now activate the special lighting effects to indicate
 * which grids are actually in the player's field of view by using dimmer
 * colors for grids which are not in the player's field of view, and/or to
 * indicate which grids are illuminated only by the player's torch by using
 * the color yellow for those grids.
 *
 * The old "update_view()" algorithm uses the special "CAVE_EASY" flag as a
 * temporary internal flag to mark those grids which are not only in view,
 * but which are also "easily" in line of sight of the player.  This flag
 * is actually just the "CAVE_SEEN" flag, and the "update_view()" function
 * makes sure to clear it for all old "CAVE_SEEN" grids, and then use it in
 * the algorithm as "CAVE_EASY", and then clear it for all "CAVE_EASY" grids,
 * and then reset it as appropriate for all new "CAVE_SEEN" grids.  This is
 * kind of messy, but it works.   The old algorithm may disappear eventually.
 *
 * The new "update_view()" algorithm uses a faster and more mathematically
 * correct algorithm, assisted by a large machine generated static array, to
 * determine the "CAVE_VIEW" and "CAVE_SEEN" flags simultaneously.  See below.
 *
 * It seems as though slight modifications to the "update_view()" functions
 * would allow us to determine "reverse" line-of-sight as well as "normal"
 * line-of-sight", which would allow monsters to have a more "correct" way
 * to determine if they can "see" the player, since right now, they "cheat"
 * somewhat and assume that if the player has "line of sight" to them, then
 * they can "pretend" that they have "line of sight" to the player.  But if
 * such a change was attempted, the monsters would actually start to exhibit
 * some undesirable behavior, such as "freezing" near the entrances to long
 * hallways containing the player, and code would have to be added to make
 * the monsters move around even if the player was not detectable, and to
 * "remember" where the player was last seen, to avoid looking stupid.
 *
 * Note that the "CAVE_GLOW" flag means that a grid is permanently lit in
 * some way.  However, for the player to "see" the grid, as determined by
 * the "CAVE_SEEN" flag, the player must not be blind, the grid must have
 * the "CAVE_VIEW" flag set, and if the grid is a "wall" grid, and it is
 * not lit by the player's torch, then it must touch a grid which does not
 * have the "CAVE_WALL" flag set, but which does have both the "CAVE_GLOW"
 * and "CAVE_VIEW" flags set.  This last part about wall grids is induced
 * by the semantics of "CAVE_GLOW" as applied to wall grids, and checking
 * the technical requirements can be very expensive, especially since the
 * grid may be touching some "illegal" grids.  Luckily, it is more or less
 * correct to restrict the "touching" grids from the eight "possible" grids
 * to the (at most) three grids which are touching the grid, and which are
 * closer to the player than the grid itself, which eliminates more than
 * half of the work, including all of the potentially "illegal" grids, if
 * at most one of the three grids is a "diagonal" grid.   In addition, in
 * almost every situation, it is possible to ignore the "CAVE_VIEW" flag
 * on these three "touching" grids, for a variety of technical reasons.
 * Finally, note that in most situations, it is only necessary to check
 * a single "touching" grid, in fact, the grid which is strictly closest
 * to the player of all the touching grids, and in fact, it is normally
 * only necessary to check the "CAVE_GLOW" flag of that grid, again, for
 * various technical reasons.  However, one of the situations which does
 * not work with this last reduction is the very common one in which the
 * player approaches an illuminated room from a dark hallway, in which the
 * two wall grids which form the "entrance" to the room would not be marked
 * as "CAVE_SEEN", since of the three "touching" grids nearer to the player
 * than each wall grid, only the farthest of these grids is itself marked
 * "CAVE_GLOW". 
 *
 *
 * Here are some pictures of the legal "light source" radius values, in
 * which the numbers indicate the "order" in which the grids could have
 * been calculated, if desired.   Note that the code will work with larger
 * radiuses, though currently yields such a radius, and the game would
 * become slower in some situations if it did.
 *
 *	 Rad=0	   Rad=1      Rad=2	   Rad=3
 *	No-Lite	 Torch,etc   Lantern	 Artifacts
 *    
 *					    333
 *			       333	   43334
 *		    212	      32123	  3321233
 *	   @	    1@1	      31@13	  331@133
 *		    212	      32123	  3321233
 *			       333	   43334
 *					    333
 *
 *
 * Here is an illustration of the two different "update_view()" algorithms,
 * in which the grids marked "%" are pillars, and the grids marked "?" are
 * not in line of sight of the player.
 *
 *
 *		      Sample situation
 *
 *		    #####################
 *		    ############.%.%.%.%#
 *		    #...@..#####........#
 *		    #............%.%.%.%#
 *		    #......#####........#
 *		    ############........#
 *		    #####################
 *
 *
 *	    New Algorithm	      Old Algorithm
 *
 *	########?????????????	 ########?????????????
 *	#...@..#?????????????	 #...@..#?????????????
 *	#...........?????????	 #.........???????????
 *	#......#####.....????	 #......####??????????
 *	########?????????...#	 ########?????????????
 *
 *	########?????????????	 ########?????????????
 *	#.@....#?????????????	 #.@....#?????????????
 *	#............%???????	 #...........?????????
 *	#......#####........?	 #......#####?????????
 *	########??????????..#	 ########?????????????
 *
 *	########?????????????	 ########?????%???????
 *	#......#####........#	 #......#####..???????
 *	#.@..........%???????	 #.@..........%???????
 *	#......#####........#	 #......#####..???????
 *	########?????????????	 ########?????????????
 *
 *	########??????????..#	 ########?????????????
 *	#......#####........?	 #......#####?????????
 *	#............%???????	 #...........?????????
 *	#.@....#?????????????	 #.@....#?????????????
 *	########?????????????	 ########?????????????
 *
 *	########?????????%???	 ########?????????????
 *	#......#####.....????	 #......####??????????
 *	#...........?????????	 #.........???????????
 *	#...@..#?????????????	 #...@..#?????????????
 *	########?????????????	 ########?????????????
 */


/*
 * The following #defines are only used for array declaration as of 
 * FAangband 0.3.4
 */

/*
 * Maximum number of grids in a single octant
 */
#define VINFO_MAX_GRIDS 161



/*
 * Maximum number of slopes in a single octant
 */
#define VINFO_MAX_SLOPES 126


/*
 * Mask of bits used in a single octant
 */
#define VINFO_BITS_3 0x3FFFFFFF
#define VINFO_BITS_2 0xFFFFFFFF
#define VINFO_BITS_1 0xFFFFFFFF
#define VINFO_BITS_0 0xFFFFFFFF

/*
 * Forward declare
 */
typedef struct vinfo_type vinfo_type;


/*
 * The 'vinfo_type' structure
 */
struct vinfo_type
{
  s16b grid_0;
  s16b grid_1;
  s16b grid_2;
  s16b grid_3;
  s16b grid_4;
  s16b grid_5;
  s16b grid_6;
  s16b grid_7;
  
  u32b bits_3;
  u32b bits_2;
  u32b bits_1;
  u32b bits_0;
  
  vinfo_type *next_0;
  vinfo_type *next_1;
  
  byte y;
  byte x;
  byte d;
  byte r;
};



/*
 * The array of "vinfo" objects, initialized by "vinfo_init()"
 */
static vinfo_type vinfo[VINFO_MAX_GRIDS];




/*
 * Slope scale factor
 */
#define SCALE 100000L


/*
 * The actual slopes (for reference)
 */

/* Bit :     Slope   Grids */
/* --- :     -----   ----- */
/*   0 :      2439	21 */
/*   1 :      2564	21 */
/*   2 :      2702	21 */
/*   3 :      2857	21 */
/*   4 :      3030	21 */
/*   5 :      3225	21 */
/*   6 :      3448	21 */
/*   7 :      3703	21 */
/*   8 :      4000	21 */
/*   9 :      4347	21 */
/*  10 :      4761	21 */
/*  11 :      5263	21 */
/*  12 :      5882	21 */
/*  13 :      6666	21 */
/*  14 :      7317	22 */
/*  15 :      7692	20 */
/*  16 :      8108	21 */
/*  17 :      8571	21 */
/*  18 :      9090	20 */
/*  19 :      9677	21 */
/*  20 :     10344	21 */
/*  21 :     11111	20 */
/*  22 :     12000	21 */
/*  23 :     12820	22 */
/*  24 :     13043	22 */
/*  25 :     13513	22 */
/*  26 :     14285	20 */
/*  27 :     15151	22 */
/*  28 :     15789	22 */
/*  29 :     16129	22 */
/*  30 :     17241	22 */
/*  31 :     17647	22 */
/*  32 :     17948	23 */
/*  33 :     18518	22 */
/*  34 :     18918	22 */
/*  35 :     20000	19 */
/*  36 :     21212	22 */
/*  37 :     21739	22 */
/*  38 :     22580	22 */
/*  39 :     23076	22 */
/*  40 :     23809	22 */
/*  41 :     24137	22 */
/*  42 :     24324	23 */
/*  43 :     25714	23 */
/*  44 :     25925	23 */
/*  45 :     26315	23 */
/*  46 :     27272	22 */
/*  47 :     28000	23 */
/*  48 :     29032	23 */
/*  49 :     29411	23 */
/*  50 :     29729	24 */
/*  51 :     30434	23 */
/*  52 :     31034	23 */
/*  53 :     31428	23 */
/*  54 :     33333	18 */
/*  55 :     35483	23 */
/*  56 :     36000	23 */
/*  57 :     36842	23 */
/*  58 :     37142	24 */
/*  59 :     37931	24 */
/*  60 :     38461	24 */
/*  61 :     39130	24 */
/*  62 :     39393	24 */
/*  63 :     40740	24 */
/*  64 :     41176	24 */
/*  65 :     41935	24 */
/*  66 :     42857	23 */
/*  67 :     44000	24 */
/*  68 :     44827	24 */
/*  69 :     45454	23 */
/*  70 :     46666	24 */
/*  71 :     47368	24 */
/*  72 :     47826	24 */
/*  73 :     48148	24 */
/*  74 :     48387	24 */
/*  75 :     51515	25 */
/*  76 :     51724	25 */
/*  77 :     52000	25 */
/*  78 :     52380	25 */
/*  79 :     52941	25 */
/*  80 :     53846	25 */
/*  81 :     54838	25 */
/*  82 :     55555	24 */
/*  83 :     56521	25 */
/*  84 :     57575	26 */
/*  85 :     57894	25 */
/*  86 :     58620	25 */
/*  87 :     60000	23 */
/*  88 :     61290	25 */
/*  89 :     61904	25 */
/*  90 :     62962	25 */
/*  91 :     63636	25 */
/*  92 :     64705	25 */
/*  93 :     65217	25 */
/*  94 :     65517	25 */
/*  95 :     67741	26 */
/*  96 :     68000	26 */
/*  97 :     68421	26 */
/*  98 :     69230	26 */
/*  99 :     70370	26 */
/* 100 :     71428	25 */
/* 101 :     72413	26 */
/* 102 :     73333	26 */
/* 103 :     73913	26 */
/* 104 :     74193	27 */
/* 105 :     76000	26 */
/* 106 :     76470	26 */
/* 107 :     77777	25 */
/* 108 :     78947	26 */
/* 109 :     79310	26 */
/* 110 :     80952	26 */
/* 111 :     81818	26 */
/* 112 :     82608	26 */
/* 113 :     84000	26 */
/* 114 :     84615	26 */
/* 115 :     85185	26 */
/* 116 :     86206	27 */
/* 117 :     86666	27 */
/* 118 :     88235	27 */
/* 119 :     89473	27 */
/* 120 :     90476	27 */
/* 121 :     91304	27 */
/* 122 :     92000	27 */
/* 123 :     92592	27 */
/* 124 :     93103	28 */
/* 125 :    100000	13 */



/*
 * Forward declare
 */
typedef struct vinfo_hack vinfo_hack;


/*
 * Temporary data used by "vinfo_init()"
 *
 *	- Number of grids
 *
 *	- Number of slopes
 *
 *	- Slope values
 *
 *	- Slope range per grid
 */
struct vinfo_hack {

  int num_slopes;
  
  long slopes[VINFO_MAX_SLOPES];
  
  long slopes_min[MAX_SIGHT_LGE + 1][MAX_SIGHT_LGE + 1];
  long slopes_max[MAX_SIGHT_LGE + 1][MAX_SIGHT_LGE + 1];
};



/*
 * Sorting hook -- comp function -- array of long's (see below)
 *
 * We use "u" to point to an array of long integers.
 */
static bool ang_sort_comp_hook_longs(vptr u, vptr v, int a, int b)
{
  long *x = (long*)(u);
  
  return (x[a] <= x[b]);
}


/*
 * Sorting hook -- comp function -- array of long's (see below)
 *
 * We use "u" to point to an array of long integers.
 */
static void ang_sort_swap_hook_longs(vptr u, vptr v, int a, int b)
{
  long *x = (long*)(u);

  long temp;
  
  /* Swap */
  temp = x[a];
  x[a] = x[b];
  x[b] = temp;
}



/*
 * Save a slope
 */
static void vinfo_init_aux(vinfo_hack *hack, int y, int x, long m)
{
  int i;
  
  /* Handle "legal" slopes */
  if ((m > 0) && (m <= SCALE))
    {
      /* Look for that slope */
      for (i = 0; i < hack->num_slopes; i++)
	{
	  if (hack->slopes[i] == m) break;
	}
      
      /* New slope */
      if (i == hack->num_slopes)
	{
	  /* Paranoia */
	  if (hack->num_slopes >= vinfo_slopes)
	    {
	      quit_fmt("Too many slopes (%d)!",
		       vinfo_slopes);
	    }
	  
	  /* Save the slope, and advance */
	  hack->slopes[hack->num_slopes++] = m;
	}
    }
  
  /* Track slope range */
  if (hack->slopes_min[y][x] > m) hack->slopes_min[y][x] = m;
  if (hack->slopes_max[y][x] < m) hack->slopes_max[y][x] = m;
}



/*
 * Initialize the "vinfo" array
 *
 * Full Octagon (radius 20), Grids=1149
 *
 * Quadrant (south east), Grids=308, Slopes=251
 *
 * Octant (east then south), Grids=161, Slopes=126
 *
 * This function assumes that vinfo_grids and vinfo_slopes
 * have the correct values, which can be derived by setting them to
 * a number which is too high, running this function, and using the
 * error messages to obtain the correct values.
 */
errr vinfo_init(void)
{
  int i, g;
  int y, x;
  
  long m;
  
  vinfo_hack *hack;
  
  int num_grids = 0;
  
  int queue_head = 0;
  int queue_tail = 0;
  vinfo_type *queue[VINFO_MAX_GRIDS*2];

  /* Set the variables for the grids, bits and slopes actually used */
  vinfo_grids  = (adult_small_device ? 48 : 161);
  vinfo_slopes = (adult_small_device ? 36 : 126);
  vinfo_bits_3 = (adult_small_device ? 0x00000000 : 0x3FFFFFFF);
  vinfo_bits_2 = (adult_small_device ? 0x00000000 : 0xFFFFFFFF);
  vinfo_bits_1 = (adult_small_device ? 0x0000000F : 0xFFFFFFFF);
  vinfo_bits_0 = (adult_small_device ? 0xFFFFFFFF : 0xFFFFFFFF);
  
  /* Make hack */
  MAKE(hack, vinfo_hack);
  
  
  /* Analyze grids */
  for (y = 0; y <= MAX_SIGHT; ++y)
    {
      for (x = y; x <= MAX_SIGHT; ++x)
	{
	  /* Skip grids which are out of sight range */
	  if (distance(0, 0, y, x) > MAX_SIGHT) continue;
	  
	  /* Default slope range */
	  hack->slopes_min[y][x] = 999999999;
	  hack->slopes_max[y][x] = 0;
	  
	  /* Paranoia */
	  if (num_grids >= vinfo_grids)
	    {
	      quit_fmt("Too many grids (%d >= %d)!",
		       num_grids, vinfo_grids);
	    }
	  
	  /* Count grids */
	  num_grids++;
	  
	  /* Slope to the top right corner */
	  m = SCALE * (1000L * y - 500) / (1000L * x + 500);
	  
	  /* Handle "legal" slopes */
	  vinfo_init_aux(hack, y, x, m);
	  
	  /* Slope to top left corner */
	  m = SCALE * (1000L * y - 500) / (1000L * x - 500);
	  
	  /* Handle "legal" slopes */
	  vinfo_init_aux(hack, y, x, m);
	  
	  /* Slope to bottom right corner */
	  m = SCALE * (1000L * y + 500) / (1000L * x + 500);
	  
	  /* Handle "legal" slopes */
	  vinfo_init_aux(hack, y, x, m);
	  
	  /* Slope to bottom left corner */
	  m = SCALE * (1000L * y + 500) / (1000L * x - 500);
	  
	  /* Handle "legal" slopes */
	  vinfo_init_aux(hack, y, x, m);
	}
    }
  
  
  /* Enforce maximal efficiency */
  if (num_grids < vinfo_grids)
    {
      quit_fmt("Too few grids (%d < %d)!",
	       num_grids, vinfo_grids);
    }
  
  /* Enforce maximal efficiency */
  if (hack->num_slopes < vinfo_slopes)
    {
      quit_fmt("Too few slopes (%d < %d)!",
	       hack->num_slopes, vinfo_slopes);
    }
  
  
  /* Sort slopes numerically */
  ang_sort_comp = ang_sort_comp_hook_longs;
  
  /* Sort slopes numerically */
  ang_sort_swap = ang_sort_swap_hook_longs;
  
  /* Sort the (unique) slopes */
  ang_sort(hack->slopes, NULL, hack->num_slopes);
  
  
  
  /* Enqueue player grid */
  queue[queue_tail++] = &vinfo[0];
  
  /* Process queue */
  while (queue_head < queue_tail)
    {
      int e;
      
      vinfo_type *p;
      
      
      /* Index */
      e = queue_head;
      
      /* Dequeue next grid */
      p = queue[queue_head++];
      
      /* Main Grid */
      g = vinfo[e].grid_0;
      
      /* Location */
      y = GRID_Y(g);
      x = GRID_X(g);
      
      
      /* Compute grid offsets */
      vinfo[e].grid_0 = GRID(+y,+x);
      vinfo[e].grid_1 = GRID(+x,+y);
      vinfo[e].grid_2 = GRID(+x,-y);
      vinfo[e].grid_3 = GRID(+y,-x);
      vinfo[e].grid_4 = GRID(-y,-x);
      vinfo[e].grid_5 = GRID(-x,-y);
      vinfo[e].grid_6 = GRID(-x,+y);
      vinfo[e].grid_7 = GRID(-y,+x);
      
      
      /* Analyze slopes */
      for (i = 0; i < hack->num_slopes; ++i)
	{
	  m = hack->slopes[i];
	  
	  /* Memorize intersection slopes (for non-player-grids) */
	  if ((e > 0) &&
	      (hack->slopes_min[y][x] < m) &&
	      (m < hack->slopes_max[y][x]))
	    {
	      switch (i / 32)
		{
		case 3: vinfo[e].bits_3 |= (1L << (i % 32)); break;
		case 2: vinfo[e].bits_2 |= (1L << (i % 32)); break;
		case 1: vinfo[e].bits_1 |= (1L << (i % 32)); break;
		case 0: vinfo[e].bits_0 |= (1L << (i % 32)); break;
		}
	    }
	}
      
      
      /* Default */
      vinfo[e].next_0 = &vinfo[0];
      
      /* Grid next child */
      if (distance(0, 0, y, x+1) <= MAX_SIGHT)
	{
	  g = GRID(y,x+1);
	  
	  if (queue[queue_tail-1]->grid_0 != g)
	    {
	      vinfo[queue_tail].grid_0 = g;
	      queue[queue_tail] = &vinfo[queue_tail];
	      queue_tail++;
	    }
	  
	  vinfo[e].next_0 = &vinfo[queue_tail - 1];
	}
      
      
      /* Default */
      vinfo[e].next_1 = &vinfo[0];
      
      /* Grid diag child */
      if (distance(0, 0, y+1, x+1) <= MAX_SIGHT)
	{
	  g = GRID(y+1,x+1);
	  
	  if (queue[queue_tail-1]->grid_0 != g)
	    {
	      vinfo[queue_tail].grid_0 = g;
	      queue[queue_tail] = &vinfo[queue_tail];
	      queue_tail++;
	    }
	  
	  vinfo[e].next_1 = &vinfo[queue_tail - 1];
	}
      
      
      /* Hack -- main diagonal has special children */
      if (y == x) vinfo[e].next_0 = vinfo[e].next_1;
      
      
      /* Extra values */
      vinfo[e].y = y;
      vinfo[e].x = x;
      vinfo[e].d = ((y > x) ? (y + x/2) : (x + y/2));
      vinfo[e].r = ((!y) ? x : (!x) ? y : (y == x) ? y : 0);
    }
  
  
  /* Verify maximal bits XXX XXX XXX */
  if (((vinfo[1].bits_3 | vinfo[2].bits_3) != vinfo_bits_3) ||
      ((vinfo[1].bits_2 | vinfo[2].bits_2) != vinfo_bits_2) ||
      ((vinfo[1].bits_1 | vinfo[2].bits_1) != vinfo_bits_1) ||
      ((vinfo[1].bits_0 | vinfo[2].bits_0) != vinfo_bits_0))
    {
      quit_fmt("%x\n%x\n%x\n%x\n", (vinfo[1].bits_3 | vinfo[2].bits_3),
	       (vinfo[1].bits_2 | vinfo[2].bits_2),
	       (vinfo[1].bits_1 | vinfo[2].bits_1), 
	       (vinfo[1].bits_0 | vinfo[2].bits_0));
    }
  
  
  /* Kill hack */
  KILL(hack);
  
  
  /* Success */
  return (0);
}



/*
 * Forget the "CAVE_VIEW" grids, redrawing as needed
 */
void forget_view(void)
{
  int i, g;
  
  int fast_view_n = view_n;
  u16b *fast_view_g = view_g;
  
  byte *fast_cave_info = &cave_info[0][0];
  
  
  /* None to forget */
  if (!fast_view_n) return;
  
  /* Clear them all */
  for (i = 0; i < fast_view_n; i++)
    {
      int y, x;
      
      /* Grid */
      g = fast_view_g[i];
      
      /* Location */
      y = GRID_Y(g);
      x = GRID_X(g);
      
      /* Clear "CAVE_VIEW" and "CAVE_SEEN" flags */
      fast_cave_info[g] &= ~(CAVE_VIEW | CAVE_SEEN);
      
      /* Only lite the spot if is on the panel (can change due to resizing */
      if (!panel_contains(y, x)) continue;
      
      /* Redraw */
      lite_spot(y, x);
    }
  
  /* None left */
  fast_view_n = 0;
  
  
  /* Save 'view_n' */
  view_n = fast_view_n;
}



/*
 * Calculate the complete field of view using a new algorithm
 *
 * If "view_g" and "temp_g" were global pointers to arrays of grids, as
 * opposed to actual arrays of grids, then we could be more efficient by
 * using "pointer swapping".
 *
 * Note the following idiom, which is used in the function below.
 * This idiom processes each "octant" of the field of view, in a
 * clockwise manner, starting with the east strip, south side,
 * and for each octant, allows a simple calculation to set "g"
 * equal to the proper grids, relative to "pg", in the octant.
 *
 *   for (o2 = 0; o2 < 16; o2 += 2)
 *   ...
 *	   g = pg + *((s16b*)(((byte*)(p))+o2));
 *   ...
 *
 *
 * Normally, vision along the major axes is more likely than vision
 * along the diagonal axes, so we check the bits corresponding to
 * the lines of sight near the major axes first.
 *
 * We use the "temp_g" array (and the "CAVE_TEMP" flag) to keep track of
 * which grids were previously marked "CAVE_SEEN", since only those grids
 * whose "CAVE_SEEN" value changes during this routine must be redrawn.
 *
 * This function is now responsible for maintaining the "CAVE_SEEN"
 * flags as well as the "CAVE_VIEW" flags, which is good, because
 * the only grids which normally need to be memorized and/or redrawn
 * are the ones whose "CAVE_SEEN" flag changes during this routine.
 *
 * Basically, this function divides the "octagon of view" into octants of
 * grids (where grids on the main axes and diagonal axes are "shared" by
 * two octants), and processes each octant one at a time, processing each
 * octant one grid at a time, processing only those grids which "might" be
 * viewable, and setting the "CAVE_VIEW" flag for each grid for which there
 * is an (unobstructed) line of sight from the center of the player grid to
 * any internal point in the grid (and collecting these "CAVE_VIEW" grids
 * into the "view_g" array), and setting the "CAVE_SEEN" flag for the grid
 * if, in addition, the grid is "illuminated" in some way.
 *
 * This function relies on a theorem (suggested and proven by Mat Hostetter)
 * which states that in each octant of a field of view, a given grid will
 * be "intersected" by one or more unobstructed "lines of sight" from the
 * center of the player grid if and only if it is "intersected" by at least
 * one such unobstructed "line of sight" which passes directly through some
 * corner of some grid in the octant which is not shared by any other octant.
 * The proof is based on the fact that there are at least three significant
 * lines of sight involving any non-shared grid in any octant, one which
 * intersects the grid and passes though the corner of the grid closest to
 * the player, and two which "brush" the grid, passing through the "outer"
 * corners of the grid, and that any line of sight which intersects a grid
 * without passing through the corner of a grid in the octant can be "slid"
 * slowly towards the corner of the grid closest to the player, until it
 * either reaches it or until it brushes the corner of another grid which
 * is closer to the player, and in either case, the existanc of a suitable
 * line of sight is thus demonstrated.
 *
 * It turns out that in each octant of the radius 20 "octagon of view",
 * there are 161 grids (with 128 not shared by any other octant), and there
 * are exactly 126 distinct "lines of sight" passing from the center of the
 * player grid through any corner of any non-shared grid in the octant.   To
 * determine if a grid is "viewable" by the player, therefore, you need to
 * simply show that one of these 126 lines of sight intersects the grid but
 * does not intersect any wall grid closer to the player.  So we simply use
 * a bit vector with 126 bits to represent the set of interesting lines of
 * sight which have not yet been obstructed by wall grids, and then we scan
 * all the grids in the octant, moving outwards from the player grid.  For
 * each grid, if any of the lines of sight which intersect that grid have not
 * yet been obstructed, then the grid is viewable.  Furthermore, if the grid
 * is a wall grid, then all of the lines of sight which intersect the grid
 * should be marked as obstructed for future reference.   Also, we only need
 * to check those grids for whom at least one of the "parents" was a viewable
 * non-wall grid, where the parents include the two grids touching the grid
 * but closer to the player grid (one adjacent, and one diagonal).  For the
 * bit vector, we simply use 4 32-bit integers.   All of the static values
 * which are needed by this function are stored in the large "vinfo" array
 * (above), which is machine generated by another program.  XXX XXX XXX
 *
 * Hack -- The queue must be able to hold more than VINFO_MAX_GRIDS grids
 * because the grids at the edge of the field of view use "grid zero" as
 * their children, and the queue must be able to hold several of these
 * special grids.  Because the actual number of required grids is bizarre,
 * we simply allocate twice as many as we would normally need.  XXX XXX XXX
 */
void update_view(void)
{
  int py = p_ptr->py;
  int px = p_ptr->px;
  
  int pg = GRID(py,px);
  
  int i, g, o2;
  
  int radius;
  
  int fast_view_n = view_n;
  u16b *fast_view_g = view_g;
  
  int fast_temp_n = 0;
  u16b *fast_temp_g = temp_g;
  
  byte *fast_cave_info = &cave_info[0][0];
  
  byte info;
  
  
  /* Hack - redetermine daylight */
  is_daylight = (((turn % (10L * TOWN_DAWN)) < ((10L * TOWN_DAWN) / 2)) && 
		 (stage_map[p_ptr->stage][STAGE_TYPE] != CAVE) &&
		 (stage_map[p_ptr->stage][STAGE_TYPE] != VALLEY) &&
		 ((p_ptr->stage < 151) || (p_ptr->stage > 153)));
    
  /*** Step 0 -- Begin ***/
  
  /* Save the old "view" grids for later */
  for (i = 0; i < fast_view_n; i++)
    {
      /* Grid */
      g = fast_view_g[i];
      
      /* Get grid info */
      info = fast_cave_info[g];
      
      /* Save "CAVE_SEEN" grids */
      if (info & (CAVE_SEEN))
	{
	  /* Set "CAVE_TEMP" flag */
	  info |= (CAVE_TEMP);
	  
	  /* Save grid for later */
	  fast_temp_g[fast_temp_n++] = g;
	}
      
      /* Clear "CAVE_VIEW" and "CAVE_SEEN" flags */
      info &= ~(CAVE_VIEW | CAVE_SEEN);
      
      /* Save cave info */
      fast_cave_info[g] = info;
    }
  
  /* Reset the "view" array */
  fast_view_n = 0;
  
  /* Extract "radius" value */
  if (is_daylight)
    radius = DUNGEON_WID;
  else if ((check_ability(SP_UNLIGHT) || p_ptr->darkness) && 
	   (p_ptr->cur_lite <= 0))
    radius = 2;
  else
    radius = p_ptr->cur_lite;
  
  /* Handle real light */
  if (radius > 0) ++radius;
  
  
  /*** Step 1 -- player grid ***/
  
  /* Player grid */
  g = pg;
  
  /* Get grid info */
  info = fast_cave_info[g];
  
  /* Assume viewable */
  info |= (CAVE_VIEW);
  
  /* Torch-lit grid */
  if (0 < radius)
    {
      /* Mark as "CAVE_SEEN" */
      info |= (CAVE_SEEN);
    }
  
  /* Perma-lit grid */
  else if (info & (CAVE_GLOW))
    {
      /* Mark as "CAVE_SEEN" */
      info |= (CAVE_SEEN);
    }
  
  /* Save cave info */
  fast_cave_info[g] = info;
  
  /* Save in array */
  fast_view_g[fast_view_n++] = g;
  
  
  /*** Step 2 -- octants ***/
  
  /* Scan each octant */
  for (o2 = 0; o2 < 16; o2 += 2)
    {
      vinfo_type *p;
      
      /* Last added */
      vinfo_type *last = &vinfo[0];
      
      /* Grid queue */
      int queue_head = 0;
      int queue_tail = 0;
      vinfo_type *queue[VINFO_MAX_GRIDS*2];
      
      /* Slope bit vector */
      u32b bits0 = vinfo_bits_0;
      u32b bits1 = vinfo_bits_1;
      u32b bits2 = vinfo_bits_2;
      u32b bits3 = vinfo_bits_3;
      
      /* Reset queue */
      queue_head = queue_tail = 0;
      
      /* Initial grids */
      queue[queue_tail++] = &vinfo[1];
      queue[queue_tail++] = &vinfo[2];
      
      /* Process queue */
      while (queue_head < queue_tail)
	{
	  /* Dequeue next grid */
	  p = queue[queue_head++];
	  
	  /* Check bits */
	  if ((bits0 & (p->bits_0)) ||
	      (bits1 & (p->bits_1)) ||
	      (bits2 & (p->bits_2)) ||
	      (bits3 & (p->bits_3)))
	    {
	      /* Extract grid value XXX XXX XXX */
	      g = pg + *((s16b*)(((byte*)(p))+o2));
	      
	      /* Get grid info */
	      info = fast_cave_info[g];
	      
	      /* Handle wall */
	      if (info & (CAVE_WALL))
		{
		  /* Clear bits */
		  bits0 &= ~(p->bits_0);
		  bits1 &= ~(p->bits_1);
		  bits2 &= ~(p->bits_2);
		  bits3 &= ~(p->bits_3);
		  
		  /* Newly viewable wall */
		  if (!(info & (CAVE_VIEW)))
		    {
		      /* Mark as viewable */
		      info |= (CAVE_VIEW);
		      
		      /* Torch-lit grids */
		      if (p->d < radius)
			{
			  /* Mark as "CAVE_SEEN" */
			  info |= (CAVE_SEEN);
			  
			}
		      
		      /* Perma-lit grids */
		      else if (info & (CAVE_GLOW))
			{
			  int y = GRID_Y(g);
			  int x = GRID_X(g);
			  
			  /* Hack -- move towards player */
			  int yy = (y < py) ? (y + 1) : (y > py) ? (y - 1) : y;
			  int xx = (x < px) ? (x + 1) : (x > px) ? (x - 1) : x;
			  
#ifdef UPDATE_VIEW_COMPLEX_WALL_ILLUMINATION
			  
			  /* Check for "complex" illumination */
			  if ((!(cave_info[yy][xx] & (CAVE_WALL)) &&
			       (cave_info[yy][xx] & (CAVE_GLOW))) ||
			      (!(cave_info[y][xx] & (CAVE_WALL)) &&
			       (cave_info[y][xx] & (CAVE_GLOW))) ||
			      (!(cave_info[yy][x] & (CAVE_WALL)) &&
			       (cave_info[yy][x] & (CAVE_GLOW))))
			    {
			      /* Mark as seen */
			      info |= (CAVE_SEEN);
			    }
			  
#else /* UPDATE_VIEW_COMPLEX_WALL_ILLUMINATION */
			  
			  /* Check for "simple" illumination */
			  if (cave_info[yy][xx] & (CAVE_GLOW))
			    {
			      /* Mark as seen */
			      info |= (CAVE_SEEN);
			    }
			  
#endif /* UPDATE_VIEW_COMPLEX_WALL_ILLUMINATION */
			  
			}
		      
		      /* Save cave info */
		      fast_cave_info[g] = info;
		      
		      /* Save in array */
		      fast_view_g[fast_view_n++] = g;
		    }
		}
	      
	      /* Handle non-wall */
	      else
		{
		  /* Enqueue child */
		  if (last != p->next_0)
		    {
		      queue[queue_tail++] = last = p->next_0;
		    }
		  
		  /* Enqueue child */
		  if (last != p->next_1)
		    {
		      queue[queue_tail++] = last = p->next_1;
		    }
		  
		  /* Newly viewable non-wall */
		  if (!(info & (CAVE_VIEW)))
		    {
		      /* Mark as "viewable" */
		      info |= (CAVE_VIEW);
		      
		      /* Torch-lit grids */
		      if (p->d < radius)
			{
			  /* Mark as "CAVE_SEEN" */
			  info |= (CAVE_SEEN);
			  
			}
		      
		      /* Perma-lit grids */
		      else if (info & (CAVE_GLOW))
			{
			  /* Mark as "CAVE_SEEN" */
			  info |= (CAVE_SEEN);
			}
		      
		      /* Save cave info */
		      fast_cave_info[g] = info;
		      
		      /* Save in array */
		      fast_view_g[fast_view_n++] = g;
		    }
		}
	    }
	}
    }
  
  
  /*** Step 3 -- Complete the algorithm ***/
  
  /* Handle blindness */
  if (p_ptr->blind)
    {
      /* Process "new" grids */
      for (i = 0; i < fast_view_n; i++)
	{
	  /* Grid */
	  g = fast_view_g[i];
	  
	  /* Grid cannot be "CAVE_SEEN" */
	  fast_cave_info[g] &= ~(CAVE_SEEN);
	}
    }
  
  /* Process "new" grids */
  for (i = 0; i < fast_view_n; i++)
    {
      /* Grid */
      g = fast_view_g[i];
      
      /* Get grid info */
      info = fast_cave_info[g];
      
      /* Was not "CAVE_SEEN", is now "CAVE_SEEN" */
      if ((info & (CAVE_SEEN)) && !(info & (CAVE_TEMP)))
	{
	  int y, x;
	  
	  /* Location */
	  y = GRID_Y(g);
	  x = GRID_X(g);
	  
	  /* Note */
	  note_spot(y, x);
	  
	  /* Redraw */
	  lite_spot(y, x);
	}
    }
  
  /* Process "old" grids */
  for (i = 0; i < fast_temp_n; i++)
    {
      /* Grid */
      g = fast_temp_g[i];
      
      /* Get grid info */
      info = fast_cave_info[g];
      
      /* Clear "CAVE_TEMP" flag */
      info &= ~(CAVE_TEMP);
      
      /* Save cave info */
      fast_cave_info[g] = info;
      
      /* Was "CAVE_SEEN", is now not "CAVE_SEEN" */
      if (!(info & (CAVE_SEEN)))
	{
	  int y, x;
	  
	  /* Location */
	  y = GRID_Y(g);
	  x = GRID_X(g);
	  
	  /* Redraw */
	  lite_spot(y, x);
	}
    }
  
  
  /* Save 'view_n' */
  view_n = fast_view_n;
}




/*
 * Every so often, the character makes enough noise that nearby 
 * monsters can use it to home in on him.
 *
 * Fill in the "cave_cost" field of every grid that the player can 
 * reach with the number of steps needed to reach that grid.  This 
 * also yields the route distance of the player from every grid.
 *
 * Monsters use this information by moving to adjacent grids with 
 * lower flow costs, thereby homing in on the player even though 
 * twisty tunnels and mazes.  Monsters can also run away from loud 
 * noises.
 *
 * The biggest limitation of this code is that it does not easily 
 * allow for alternate ways around doors (not all monsters can handle 
 * doors) and lava/water (many monsters are not allowed to enter 
 * water, lava, or both).
 *
 * The flow table is three-dimensional.  The first dimension allows the 
 * table to both store and overwrite grids safely.  The second indicates 
 * whether this value is that for x or for y.  The third is the number 
 * of grids able to be stored at any flow distance.
 */
void update_noise(void)
{
#ifdef MONSTER_FLOW
  int cost;
  int route_distance = 0;
  
  int i, d;
  int y, x, y2, x2;
  int last_index;
  int grid_count = 0;
  
  int dist;
  bool full = FALSE;
  
  /* Note where we get information from, and where we overwrite */
  int this_cycle = 0;
  int next_cycle = 1;
  
  byte flow_table[2][2][8 * NOISE_STRENGTH];
  
  /* The character's grid has no flow info.  Do a full rebuild. */
  if (cave_cost[p_ptr->py][p_ptr->px] == 0) full = TRUE;
  
  /* Determine when to rebuild, update, or do nothing */
  if (!full)
    {
      dist = ABS(p_ptr->py - flow_center_y);
      if (ABS(p_ptr->px - flow_center_x) > dist)
	dist = ABS(p_ptr->px - flow_center_x);
      
      /*
       * Character is far enough away from the previous flow center - 
       * do a full rebuild.
       */
      if (dist >= 15) full = TRUE;
      
      else
	{
	  /* Get axis distance to center of last update */
	  dist = ABS(p_ptr->py - update_center_y);
	  if (ABS(p_ptr->px - update_center_x) > dist)
	    dist = ABS(p_ptr->px - update_center_x);
	  
	  /*
	   * We probably cannot decrease the center cost any more.
	   * We should assume that we have to do a full rebuild.
	   */
	  if (cost_at_center - (dist + 5) <= 0) full = TRUE;
	  
	  
	  /* Less than five grids away from last update */
	  else if (dist < 5)
	    {
	      /* We're in LOS of the last update - don't update again */
	      if (los(p_ptr->py, p_ptr->px, update_center_y, 
		      update_center_x)) return;
	      
	      /* We're not in LOS - update */
	      else full = FALSE;
	    }
	  
	  /* Always update if at least five grids away */
	  else full = FALSE;
	}
    }
  
  /* Update */
  if (!full)
    {
      bool found = FALSE;
      
      /* Start at the character's location */
      flow_table[this_cycle][0][0] = p_ptr->py;
      flow_table[this_cycle][1][0] = p_ptr->px;
      grid_count = 1;
      
      /* Erase outwards until we hit the previous update center */
      for (cost = 0; cost <= NOISE_STRENGTH; cost++)
	{
	  /*
	   * Keep track of the route distance to the previous 
	   * update center.
	   */
	  route_distance++;
	  
	  
	  /* Get the number of grids we'll be looking at */
	  last_index = grid_count;
	  
	  /* Clear the grid count */
	  grid_count = 0;
	  
	  /* Get each valid entry in the flow table in turn */
	  for (i = 0; i < last_index; i++)
	    {
	      /* Get this grid */
	      y = flow_table[this_cycle][0][i];
	      x = flow_table[this_cycle][1][i];
	      
	      /* Look at all adjacent grids */
	      for (d = 0; d < 8; d++)
		{
		  /* Child location */
		  y2 = y + ddy_ddd[d];
		  x2 = x + ddx_ddd[d];
		  
		  /* Check Bounds */
		  if (!in_bounds(y2, x2)) continue;
		  
		  /* Ignore illegal grids */
		  if (cave_cost[y2][x2] == 0) continue;
		  
		  /* Ignore previously erased grids */
		  if (cave_cost[y2][x2] == 255) continue;
		  
		  /* Erase previous info, mark grid */
		  cave_cost[y2][x2] = 255;
		  
		  /* Store this grid in the flow table */
		  flow_table[next_cycle][0][grid_count] = y2;
		  flow_table[next_cycle][1][grid_count] = x2;
		  
		  /* Increment number of grids stored */
		  grid_count++;
		  
		  /* If this is the previous update center, we can stop */
		  if ((y2 == update_center_y) && 
		      (x2 == update_center_x)) found = TRUE;
		}
	    }
	  
	  /* Stop when we find the previous update center. */
	  if (found) break;
	  
	  
	  /* Swap write and read portions of the table */
	  if (this_cycle == 0)
	    {
	      this_cycle = 1;
	      next_cycle = 0;
	    }
	  else
	    {
	      this_cycle = 0;
	      next_cycle = 1;
	    }
	}
      
      /*
       * Reduce the flow cost assigned to the new center grid by 
       * enough to maintain the correct cost slope out to the range 
       * we have to update the flow.
       */
      cost_at_center -= route_distance;
      
      /* We can't reduce the center cost any more.  Do a full rebuild. */
      if (cost_at_center < 0) full = TRUE;
      
      else
	{
	  /* Store the new update center */
	  update_center_y = p_ptr->py;
	  update_center_x = p_ptr->px;
	}
    }
  
  
  /* Full rebuild */
  if (full)
    {
      /*
       * Set the initial cost to 100; updates will progressively 
       * lower this value.  When it reaches zero, another full 
       * rebuild has to be done.
       */
      cost_at_center = 100;
      
      /* Save the new noise epicenter */
      flow_center_y = p_ptr->py;
      flow_center_x = p_ptr->px;
      update_center_y = p_ptr->py;
      update_center_x = p_ptr->px;
      
      
      /* Erase all of the current flow (noise) information */
      for (y = 0; y < DUNGEON_HGT; y++)
	{
	  for (x = 0; x < DUNGEON_WID; x++)
	    {
	      cave_cost[y][x] = 0;
	    }
	}
    }
  
  
  /*** Update or rebuild the flow ***/
  
  
  /* Store base cost at the character location */
  cave_cost[p_ptr->py][p_ptr->px] = cost_at_center;
  
  /* Store this grid in the flow table, note that we've done so */
  flow_table[this_cycle][0][0] = p_ptr->py;
  flow_table[this_cycle][1][0] = p_ptr->px;
  grid_count = 1;
  
  /* Extend the noise burst out to its limits */
  for (cost = cost_at_center + 1; cost <= cost_at_center + NOISE_STRENGTH; 
       cost++)
    {
      /* Get the number of grids we'll be looking at */
      last_index = grid_count;
      
      /* Stop if we've run out of work to do */
      if (last_index == 0) break;
      
      /* Clear the grid count */
      grid_count = 0;
      
      /* Get each valid entry in the flow table in turn. */
      for (i = 0; i < last_index; i++)
	{
	  /* Get this grid */
	  y = flow_table[this_cycle][0][i];
	  x = flow_table[this_cycle][1][i];
	  
	  /* Look at all adjacent grids */
	  for (d = 0; d < 8; d++)
	    {
	      /* Child location */
	      y2 = y + ddy_ddd[d];
	      x2 = x + ddx_ddd[d];
	      
	      /* Check Bounds */
	      if (!in_bounds(y2, x2)) continue;
	      
	      /* When doing a rebuild... */
	      if (full)
		{
		  /* Ignore previously marked grids */
		  if (cave_cost[y2][x2]) continue;
		  
		  /* Ignore walls.  Do not ignore rubble. */
		  if ((cave_feat[y2][x2] > FEAT_RUBBLE) && 
		      (cave_feat[y2][x2] < FEAT_SHOP_HEAD)) 
		    {
		      continue;
		    }
		}
	      
	      /* When doing an update... */
	      else
		{
		  /* Ignore all but specially marked grids */
		  if (cave_cost[y2][x2] != 255) continue;
		}
	      
	      /* Store cost at this location */
	      cave_cost[y2][x2] = cost;
	      
	      /* Store this grid in the flow table */
	      flow_table[next_cycle][0][grid_count] = y2;
	      flow_table[next_cycle][1][grid_count] = x2;
	      
	      /* Increment number of grids stored */
	      grid_count++;
	    }
	}
      
      /* Swap write and read portions of the table */
      if (this_cycle == 0)
	{
	  this_cycle = 1;
	  next_cycle = 0;
	}
      else
	{
	  this_cycle = 0;
	  next_cycle = 1;
	}
    }
  
#endif
}


/*
 * Characters leave scent trails for perceptive monsters to track.
 *
 * Smell is rather more limited than sound.  Many creatures cannot use 
 * it at all, it doesn't extend very far outwards from the character's 
 * current position, and monsters can use it to home in the character, 
 * but not to run away from him.
 *
 * Smell is valued according to age.  When a character takes his turn, 
 * scent is aged by one, and new scent of the current age is laid down.  
 * Speedy characters leave more scent, true, but it also ages faster, 
 * which makes it harder to hunt them down.
 *
 * Whenever the age count loops, most of the scent trail is erased and 
 * the age of the remainder is recalculated.
 */
void update_smell(void)
{
#ifdef MONSTER_FLOW
  
  int i, j;
  int y, x;
  
  int py = p_ptr->py;
  int px = p_ptr->px;

  feature_type *f_ptr = NULL;  
  
  /* Create a table that controls the spread of scent */
  int scent_adjust[5][5] = 
    {
      { 250,  2,  2,  2, 250 },
      {   2,  1,  1,  1,   2 },
      {   2,  1,  0,  1,   2 },
      {   2,  1,  1,  1,   2 },
      { 250,  2,  2,  2, 250 },
    };
  
  /* Scent becomes "younger" */
  scent_when--;
  
  /* Loop the age and adjust scent values when necessary */
  if (scent_when <= 0)
    {
      /* Scan the entire dungeon */
      for (y = 0; y < DUNGEON_HGT; y++)
	{
	  for (x = 0; x < DUNGEON_WID; x++)
	    {
	      /* Ignore non-existent scent */
	      if (cave_when[y][x] == 0) continue;
	      
	      /* Erase the earlier part of the previous cycle */
	      if (cave_when[y][x] > SMELL_STRENGTH) cave_when[y][x] = 0;
	      
	      /* Reset the ages of the most recent scent */
	      else cave_when[y][x] = 250 - SMELL_STRENGTH + cave_when[y][x];
	    }
	}
      
      /* Reset the age value */
      scent_when = 250 - SMELL_STRENGTH;
    }
  
  
  /* Lay down new scent */
  for (i = 0; i < 5; i++)
    {
      for (j = 0; j < 5; j++)
	{
	  /* Translate table to map grids */
	  y = i + py - 2;
	  x = j + px - 2;

	  /* Check Bounds */
	  if (!in_bounds(y, x)) continue;
	  
	  /* Get the feature */
	  f_ptr = &f_info[cave_feat[y][x]];
	  
	  /* Walls, water, and lava cannot hold scent. */
	  if (f_ptr->flags & TF_NO_SCENT)
	    {
	      continue;
	    }
	  
	  /* Grid must not be blocked by walls from the character */
	  if (!los(p_ptr->py, p_ptr->px, y, x)) continue;
	  
	  /* Note grids that are too far away */
	  if (scent_adjust[i][j] == 250) continue;
	  
	  /* Mark the grid with new scent */
	  cave_when[y][x] = scent_when + scent_adjust[i][j];
	}
    }
  
#endif
}

/*
 * Map around a given point, or the current panel (plus some) 
 * ala "magic mapping".   Staffs of magic mapping map more than 
 * rods do, because staffs affect larger areas in general.
 *
 * We must never attempt to map the outer dungeon walls, or we
 * might induce illegal cave grid references.
 */
void map_area(int y, int x, bool extended)
{
  int i, y_c, x_c;
  int rad=DETECT_RAD_DEFAULT;
  
  if (extended) rad += 10;
  
  /* Map around a location, if given. */
  if ((y) && (x))
    {
      y_c = y;
      x_c = x;
    }
  
  /* Normally, pick an area to map around the player */
  else
    {
      y_c = p_ptr->py;
      x_c = p_ptr->px;
    }
  
  /* Scan the maximal area of mapping */
  for (y = y_c - rad; y <= y_c + rad; y++)
    {
      for (x = x_c - rad; x <= x_c + rad; x++)
	{
	  
	  /* Ignore "illegal" locations */
	  if (!in_bounds(y, x)) continue;
	  
	  /* Enforce a "circular" area */
	  if (distance(y_c, x_c, y, x) > rad) continue;
	  
	  /* All non-walls, trees, and rubble are "checked" */
	  if ((cave_feat[y][x] < FEAT_SECRET) ||
	      (cave_feat[y][x] == FEAT_RUBBLE) ||
	      (cave_feat[y][x] >= FEAT_LAVA))
	    {
	      /* Memorize normal features */
	      if (cave_feat[y][x] > FEAT_INVIS)
		{
		  /* Memorize the object */
		  cave_info[y][x] |= (CAVE_MARK);
		}
	      
	      /* Memorize known walls */
	      for (i = 0; i < 8; i++)
		{
		  int yy = y + ddy_ddd[i];
		  int xx = x + ddx_ddd[i];
		  
		  /* All walls are "checked" */
		  if (cave_feat[yy][xx] >= FEAT_SECRET) 
		    {
		      /* Memorize the walls */
		      cave_info[yy][xx] |= (CAVE_MARK);
		    }
		}
	    }
	}
    }
  
  /* Redraw map */
  p_ptr->redraw |= (PR_MAP);
  
  /* Window stuff */
  p_ptr->window |= (PW_OVERHEAD);
}



/*
 * Light up the dungeon using "claravoyance"
 *
 * This function "illuminates" every grid in the dungeon, memorizes all
 * "objects", memorizes all grids as with magic mapping, and, under the
 * standard option settings (view_perma_grids but not view_torch_grids)
 * memorizes all floor grids too.
 *
 * In Oangband, greater and lesser vaults only become fully known if the 
 * player has accessed this function from the debug commands.  Otherwise, 
 * they act like magically mapped permenantly lit rooms.
 *
 * Note that if "view_perma_grids" is not set, we do not memorize floor
 * grids, since this would defeat the purpose of "view_perma_grids", not
 * that anyone seems to play without this option.
 *
 * Note that if "view_torch_grids" is set, we do not memorize floor grids,
 * since this would prevent the use of "view_torch_grids" as a method to
 * keep track of what grids have been observed directly.
 */
void wiz_lite(bool wizard)
{
  int i, y, x;  
  
  /* Memorize objects */
  for (i = 1; i < o_max; i++)
    {
      object_type *o_ptr = &o_list[i];
      
      /* Skip dead objects */
      if (!o_ptr->k_idx) continue;
      
      /* Skip held objects */
      if (o_ptr->held_m_idx) continue;
      
      /* Skip objects in vaults, if not a wizard. */
      if ((wizard == FALSE) && 
	  (cave_info[o_ptr->iy][o_ptr->ix] & (CAVE_ICKY))) continue;
      
      /* Memorize */
      o_ptr->marked = TRUE;
    }
  
  /* Scan all normal grids */
  for (y = 1; y < DUNGEON_HGT-1; y++)
    {
      /* Scan all normal grids */
      for (x = 1; x < DUNGEON_WID-1; x++)
	{
	  /* Process all non-walls, trees, and rubble. */
	  if ((cave_feat[y][x] < FEAT_SECRET) || 
	      (cave_feat[y][x] == FEAT_RUBBLE)||
	      (cave_feat[y][x] >= FEAT_LAVA))
	    {
	      /* Scan all neighbors */
	      for (i = 0; i < 9; i++)
		{
		  int yy = y + ddy_ddd[i];
		  int xx = x + ddx_ddd[i];
		  
		  /* Perma-lite the grid */
		  cave_info[yy][xx] |= (CAVE_GLOW);
		  
		  /* Skip non-wall vault features if not a wizard. */
		  if ((wizard == FALSE) && 
		      (cave_info[yy][xx] & (CAVE_ICKY)) && 
		      ((cave_feat[yy][xx] < FEAT_SECRET) || 
		       (cave_feat[yy][xx] > FEAT_PERM_SOLID))) continue;
		  
		  /* Memorize normal features */
		  if (cave_feat[yy][xx] > FEAT_INVIS)
		    {
		      /* Memorize the grid */
		      cave_info[yy][xx] |= (CAVE_MARK);
		    }
		  
		  /* Normally, memorize floors (see above). */
		  if (view_perma_grids && !view_torch_grids)
		    {
		      /* Memorize the grid */
		      cave_info[yy][xx] |= (CAVE_MARK);
		    }
		}
	    }
	}
    }
  
  /* Fully update the visuals */
  p_ptr->update |= (PU_FORGET_VIEW | PU_UPDATE_VIEW | PU_MONSTERS);
  
  /* Redraw map */
  p_ptr->redraw |= (PR_MAP);
  
  /* Window stuff */
  p_ptr->window |= (PW_OVERHEAD | PW_MONLIST );
}



/*
 * Forget the dungeon map (ala "Thinking of Maud...").
 */
void wiz_dark(void)
{
  int i, y, x;
  
  
  /* Forget every grid */
  for (y = 0; y < DUNGEON_HGT; y++)
    {
      for (x = 0; x < DUNGEON_WID; x++)
	{
	  /* Process the grid */
	  cave_info[y][x] &= ~(CAVE_MARK);
	  cave_info2[y][x] &= ~(CAVE2_DTRAP);
	}
    }
  
  /* Forget all objects */
  for (i = 1; i < o_max; i++)
    {
      object_type *o_ptr = &o_list[i];
      
      /* Skip dead objects */
      if (!o_ptr->k_idx) continue;
      
      /* Skip held objects */
      if (o_ptr->held_m_idx) continue;
      
      /* Forget the object */
      o_ptr->marked = FALSE;
    }
  
  /* Fully update the visuals */
  p_ptr->update |= (PU_FORGET_VIEW | PU_UPDATE_VIEW | PU_MONSTERS);
  
  /* Redraw map */
  p_ptr->redraw |= (PR_MAP);
  
  /* Window stuff */
  p_ptr->window |= (PW_OVERHEAD | PW_MONLIST);
}



/*
 * Light or Darken the town
 */
void town_illuminate(bool daytime, bool cave)
{
  int y, x, i;
  
  
  /* Apply light or darkness */
  for (y = 0; y < DUNGEON_HGT; y++)
    {
      for (x = 0; x < DUNGEON_WID; x++)
	{
	  /* Grids "outside" the town walls */
	  if (cave_feat[y][x] == FEAT_PERM_SOLID) {
	    
	    /* Darken the grid */
	    cave_info[y][x] &= ~(CAVE_GLOW);
	    
	    /* Hack -- Forget grids */
	    if (view_perma_grids)
	      {
		cave_info[y][x] &= ~(CAVE_MARK);
	      }
	  }
	  
	  /* Interesting grids */
	  else if (cave_feat[y][x] > FEAT_INVIS)
	    {
	      /* Illuminate the grid */
	      cave_info[y][x] |= (CAVE_GLOW);
	      
	      /* Memorize the grid */
	      cave_info[y][x] |= (CAVE_MARK);
	    }
	  
	  /* Boring grids (light) */
	  else if (daytime && (!cave))
	    {
	      /* Illuminate the grid */
	      cave_info[y][x] |= (CAVE_GLOW);
	      
	      /* Hack -- Memorize grids */
	      if (view_perma_grids)
		{
		  cave_info[y][x] |= (CAVE_MARK);
		}
	    }
	  
	  /* Boring grids (dark) */
	  else
	    {
	      /* Darken the grid */
	      cave_info[y][x] &= ~(CAVE_GLOW);
	      
	      /* Hack -- Forget grids */
	      if (view_perma_grids)
		{
		  cave_info[y][x] &= ~(CAVE_MARK);
		}
	    }
	}
    }
  
  
  /* Handle shop doorways */
  for (y = 0; y < DUNGEON_HGT; y++)
    {
      for (x = 0; x < DUNGEON_WID; x++)
	{
	  /* Track shop doorways */
	  if ((cave_feat[y][x] >= FEAT_SHOP_HEAD) &&
	      (cave_feat[y][x] <= FEAT_SHOP_TAIL))
	    {
	      for (i = 0; i < 8; i++)
		{
		  int yy = y + ddy_ddd[i];
		  int xx = x + ddx_ddd[i];
		  
		  /* Illuminate the grid */
		  cave_info[yy][xx] |= (CAVE_GLOW);
		  
		  /* Hack -- Memorize grids */
		  if (view_perma_grids)
		    {
		      cave_info[yy][xx] |= (CAVE_MARK);
		    }
		}
	    }
	}
    }
  
  
  /* Fully update the visuals */
  p_ptr->update |= (PU_FORGET_VIEW | PU_UPDATE_VIEW | PU_MONSTERS);
  
  /* Redraw map */
  p_ptr->redraw |= (PR_MAP);
  
  /* Window stuff */
  p_ptr->window |= (PW_OVERHEAD | PW_MONLIST);
}


/*
 * Change the "feat" flag for a grid, and notice/redraw the grid. 
 */
void cave_set_feat(int y, int x, int feat)
{

  feature_type *f_ptr = &f_info[feat];

  /* Change the feature */
  cave_feat[y][x] = feat;
  
  /* Handle "floor" grids. */
  if ((f_ptr->flags & TF_LOS) || (f_ptr->flags & TF_SHOP))
    {
      cave_info[y][x] &= ~(CAVE_WALL);
    }
  
  /* Handle "wall"/etc grids */
  else
    {
      cave_info[y][x] |= (CAVE_WALL);
    }
  
  /* Notice/Redraw */
  if (character_dungeon)
    {
      /* Notice */
      note_spot(y, x);
      
      /* Redraw */
      lite_spot(y, x);
    }
}



/*
 * Determine the path taken by a projection.
 *
 * The projection will always start from the grid (y1,x1), and will travel
 * towards the grid (y2,x2), touching one grid per unit of distance along
 * the major axis, and stopping when it enters the destination grid or a
 * wall grid, or has travelled the maximum legal distance of "range".
 *
 * Note that "distance" in this function (as in the "update_view()" code)
 * is defined as "MAX(dy,dx) + MIN(dy,dx)/2", which means that the player
 * actually has an "octagon of projection" not a "circle of projection".
 *
 * The path grids are saved into the grid array pointed to by "gp", and
 * there should be room for at least "range" grids in "gp".  Note that
 * due to the way in which distance is calculated, this function normally
 * uses fewer than "range" grids for the projection path, so the result
 * of this function should never be compared directly to "range".  Note
 * that the initial grid (y1,x1) is never saved into the grid array, not
 * even if the initial grid is also the final grid.  XXX XXX XXX
 *
 * The "flg" flags can be used to modify the behavior of this function.
 *
 * In particular, the "PROJECT_STOP" and "PROJECT_THRU" flags have the same
 * semantics as they do for the "project" function, namely, that the path
 * will stop as soon as it hits a monster, or that the path will continue
 * through the destination grid, respectively.
 *
 * The "PROJECT_JUMP" flag, which for the "project()" function means to
 * start at a special grid (which makes no sense in this function), means
 * that the path should be "angled" slightly if needed to avoid any wall
 * grids, allowing the player to "target" any grid which is in "view".
 * This flag is non-trivial and has not yet been implemented, but could
 * perhaps make use of the "vinfo" array (above).  XXX XXX XXX
 *
 * This function returns the number of grids (if any) in the path.  This
 * function will return zero if and only if (y1,x1) and (y2,x2) are equal.
 *
 * This algorithm is similar to, but slightly different from, the one used
 * by "update_view_los()", and very different from the one used by "los()".
 */
sint project_path(u16b *gp, int range, int y1, int x1, int y2, int x2, int flg)
{
  int y, x;
  
  int n = 0;
  int k = 0;
  
  /* Absolute */
  int ay, ax;
  
  /* Offsets */
  int sy, sx;
  
  /* Fractions */
  int frac;
  
  /* Scale factors */
  int full, half;
  
  /* Slope */
  int m;
  
  bool blocked=FALSE;
  
  /* No path necessary (or allowed) */
  if ((x1 == x2) && (y1 == y2)) return (0);
  
  
  /* Analyze "dy" */
  if (y2 < y1)
    {
      ay = (y1 - y2);
      sy = -1;
    }
  else
    {
      ay = (y2 - y1);
      sy = 1;
    }
  
  /* Analyze "dx" */
  if (x2 < x1)
    {
      ax = (x1 - x2);
      sx = -1;
    }
  else
    {
      ax = (x2 - x1);
      sx = 1;
    }
  
  
  /* Number of "units" in one "half" grid */
  half = (ay * ax);
  
  /* Number of "units" in one "full" grid */
  full = half << 1;
  
  
  /* Vertical */
  if (ay > ax)
    {
      /* Start at tile edge */
      frac = ax * ax;
      
      /* Let m = ((dx/dy) * full) = (dx * dx * 2) = (frac * 2) */
      m = frac << 1;
      
      /* Start */
      y = y1 + sy;
      x = x1;
      
      /* Create the projection path */
      while (1)
	{
	  /* Save grid */
	  gp[n++] = GRID(y,x);
	  
	  /* Hack -- Check maximum range */
	  if ((n + (k >> 1)) >= range) break;
	  
	  /* Sometimes stop at destination grid */
	  if (!(flg & (PROJECT_THRU)))
	    {
	      if ((x == x2) && (y == y2)) break;
	    }
	  
	  /* Always stop at non-initial wall grids */
	  if ((n > 0) && !cave_floor_bold(y, x)) break;
	  
	  /* Sometimes stop at non-initial monsters/players */
	  if (flg & (PROJECT_STOP))
	    {
	      if ((n > 0) && (cave_m_idx[y][x] != 0)) break;
	    }
	  
	  /* Sometimes notice non-initial monsters/players */
	  if (flg & (PROJECT_CHCK))
	    {
	      if ((n > 0) && (cave_m_idx[y][x] != 0)) blocked = TRUE;
	    }
	  
	  /* Slant */
	  if (m)
	    {
	      /* Advance (X) part 1 */
	      frac += m;
	      
	      /* Horizontal change */
	      if (frac >= half)
		{
		  /* Advance (X) part 2 */
		  x += sx;
		  
		  /* Advance (X) part 3 */
		  frac -= full;
		  
		  /* Track distance */
		  k++;
		}
	    }
	  
	  /* Advance (Y) */
	  y += sy;
	}
    }

  /* Horizontal */
  else if (ax > ay)
    {
      /* Start at tile edge */
      frac = ay * ay;
      
      /* Let m = ((dy/dx) * full) = (dy * dy * 2) = (frac * 2) */
      m = frac << 1;
      
      /* Start */
      y = y1;
      x = x1 + sx;
      
      /* Create the projection path */
      while (1)
	{
	  /* Save grid */
	  gp[n++] = GRID(y,x);
	  
	  /* Hack -- Check maximum range */
	  if ((n + (k >> 1)) >= range) break;
	  
	  /* Sometimes stop at destination grid */
	  if (!(flg & (PROJECT_THRU)))
	    {
	      if ((x == x2) && (y == y2)) break;
	    }
	  
	  /* Always stop at non-initial wall grids */
	  if ((n > 0) && !cave_floor_bold(y, x)) break;
	  
	  /* Sometimes stop at non-initial monsters/players */
	  if (flg & (PROJECT_STOP))
	    {
	      if ((n > 0) && (cave_m_idx[y][x] != 0)) break;
	    }
	  
	  /* Sometimes notice non-initial monsters/players */
	  if (flg & (PROJECT_CHCK))
	    {
	      if ((n > 0) && (cave_m_idx[y][x] != 0)) blocked = TRUE;
	    }
	  
	  /* Slant */
	  if (m)
	    {
	      /* Advance (Y) part 1 */
	      frac += m;
	      
	      /* Vertical change */
	      if (frac >= half)
		{
		  /* Advance (Y) part 2 */
		  y += sy;
		  
		  /* Advance (Y) part 3 */
		  frac -= full;
		  
		  /* Track distance */
		  k++;
		}
	    }
	  
	  /* Advance (X) */
	  x += sx;
	}
    }
  
  /* Diagonal */
  else
    {
      /* Start */
      y = y1 + sy;
      x = x1 + sx;
      
      /* Create the projection path */
      while (1)
	{
	  /* Save grid */
	  gp[n++] = GRID(y,x);
	  
	  /* Hack -- Check maximum range */
	  if ((n + (n >> 1)) >= range) break;
	  
	  /* Sometimes stop at destination grid */
	  if (!(flg & (PROJECT_THRU)))
	    {
	      if ((x == x2) && (y == y2)) break;
	    }
	  
	  /* Always stop at non-initial wall grids */
	  if ((n > 0) && !cave_floor_bold(y, x)) break;
	  
	  /* Sometimes stop at non-initial monsters/players */
	  if (flg & (PROJECT_STOP))
	    {
	      if ((n > 0) && (cave_m_idx[y][x] != 0)) break;
	    }
	  
	  /* Sometimes notice non-initial monsters/players */
	  if (flg & (PROJECT_CHCK))
	    {
	      if ((n > 0) && (cave_m_idx[y][x] != 0)) blocked = TRUE;
	    }
	  
	  /* Advance (Y) */
	  y += sy;
	  
	  /* Advance (X) */
	  x += sx;
	}
    }
  
  
  /* Length */
  if (blocked) return(-n);
  else return (n);
}


/*
 * Determine if a bolt spell cast from (y1,x1) to (y2,x2) will arrive
 * at the final destination, using the "project_path()" function to check 
 * the projection path.
 *
 * Accept projection flags, and pass them onto "project_path()".
 *
 * Note that no grid is ever "projectable()" from itself.
 *
 * This function is used to determine if the player can (easily) target
 * a given grid, if a monster can target the player, and if a clear shot 
 * exists from monster to player.
 */
byte projectable(int y1, int x1, int y2, int x2, int flg)
{
  int y, x;
  
  int grid_n = 0;
  u16b grid_g[512];
  
  /* Check the projection path */
  grid_n = project_path(grid_g, MAX_RANGE, y1, x1, y2, x2, flg);
  
  /* No grid is ever projectable from itself */
  if (!grid_n) return (FALSE);
  
  /* Final grid.  As grid_n may be negative, use absolute value.  */
  y = GRID_Y(grid_g[ABS(grid_n) - 1]);
  x = GRID_X(grid_g[ABS(grid_n) - 1]);
  
  /* May not end in an unrequested grid */
  if ((y != y2) || (x != x2)) return (PROJECT_NO);
  
  /* May not end in a wall, unless a tree or rubble grid. */
  if (!cave_passable_bold(y, x)) return (PROJECT_NO);
  
  
  /* Promise a clear bolt shot if we have verified that there is one */
  if ((flg & (PROJECT_STOP)) || (flg & (PROJECT_CHCK)))
    {
      /* Positive value for grid_n mean no obstacle was found. */
      if (grid_n > 0) return (PROJECT_CLEAR);
    }
  
  /* Assume projectable, but make no promises about clear shots */
  return (PROJECT_NOT_CLEAR);
}

/*
 * Standard "find me a location" function
 *
 * Obtains a legal location within the given distance of the initial
 * location, and with "los()" from the source to destination location.
 *
 * This function is often called from inside a loop which searches for
 * locations while increasing the "d" distance.
 *
 * Currently the "m" parameter is unused.
 */
void scatter(int *yp, int *xp, int y, int x, int d, int m)
{
  int nx, ny;
  
  /* Unused */
  m = m;
  
  /* Pick a location */
  while (TRUE)
    {
      /* Pick a new location */
      ny = rand_spread(y, d);
      nx = rand_spread(x, d);
      
      /* Ignore annoying locations */
      if (!in_bounds_fully(y, x)) continue;
      
      /* Ignore "excessively distant" locations */
      if ((d > 1) && (distance(y, x, ny, nx) > d)) continue;
      
      /* Require "line of sight" */
      if (los(y, x, ny, nx)) break;
    }
  
  /* Save the location */
  (*yp) = ny;
  (*xp) = nx;
}






/*
 * Track a new monster
 */
void health_track(int m_idx)
{
  /* Track a new guy */
  p_ptr->health_who = m_idx;
  
  /* Redraw (later) */
  p_ptr->redraw |= (PR_HEALTH | PR_MON_MANA);
}



/*
 * Hack -- track the given monster race
 */
void monster_race_track(int r_idx)
{
  /* Save this monster ID */
  p_ptr->monster_race_idx = r_idx;
  
  /* Window stuff */
  p_ptr->window |= (PW_MONSTER);
}



/*
 * Hack -- track the given object kind
 */
void object_kind_track(int k_idx)
{
  /* Save this object ID */
  p_ptr->object_kind_idx = k_idx;
  
  /* Window stuff */
  p_ptr->window |= (PW_OBJECT);
}



/*
 * Something has happened to disturb the player.
 *
 * The first arg indicates a major disturbance, which affects search.
 *
 * The second arg is currently unused, but could induce output flush.
 *
 * All disturbance cancels repeated commands, resting, and running.
 */
void disturb(int stop_search, int unused_flag)
{
  /* Cancel auto-commands */
  /* p_ptr->command_new = 0; */
  
  /* Cancel repeated commands */
  if (p_ptr->command_rep)
    {
      /* Cancel */
      p_ptr->command_rep = 0;
      
      /* Redraw the state (later) */
      p_ptr->redraw |= (PR_STATE);
    }
  
  /* Cancel Resting */
  if (p_ptr->resting)
    {
      /* Cancel */
      p_ptr->resting = 0;
      
      /* Redraw the state (later) */
      p_ptr->redraw |= (PR_STATE);
    }
  
  /* Cancel running */
  if (p_ptr->running)
    {
      /* Cancel */
      p_ptr->running = 0;
      
      /* Recenter the panel when running stops */
      if (center_player && !center_running ) verify_panel();
      
      /* Calculate torch radius */
      p_ptr->update |= (PU_TORCH);
      
      /* Redraw the player */
      if (hidden_player)
	{
	  int py = p_ptr->py;
	  int px = p_ptr->px;
	  
	  /* Redraw player */
	  lite_spot(py, px);
	}
    }
  
  /* Cancel searching if requested */
  if (stop_search && p_ptr->searching)
    {
      /* Cancel */
      p_ptr->searching = FALSE;
      
      /* Recalculate bonuses */
      p_ptr->update |= (PU_BONUS);
      
      /* Redraw the state */
      p_ptr->redraw |= (PR_STATE);
    }
  
  /* Flush the input if requested */
  if (flush_disturb) flush();
}




/*
 * Hack -- Check if a level is a "quest" level
 */
bool is_quest(int stage)
{
  int i;
  
  /* Check quests */
  for (i = 0; i < MAX_Q_IDX; i++)
    {
      /* Check for quest */
      if (q_list[i].stage == stage) return (TRUE);
    }
  
  /* Nope */
  return (FALSE);
}





