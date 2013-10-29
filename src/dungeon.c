/* File: dungeon.c */

/*
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies.  Other copyrights may also apply.
 */

#include "angband.h"
#include "z-file.h"
#include "cmds.h"
#include "script.h"

#ifdef EFG
/* EFGchange allow pseudo on jewelry */
typedef byte Bool;
static Bool excellent_jewelry_p(const object_type *o_ptr)
{
	/* some rings and amulets are considered excellent without an ego */
	if (o_ptr->tval == TV_RING)
	{
		switch(o_ptr->sval)
		{
			case SV_RING_RESIST_FIRE:
			case SV_RING_RESIST_COLD:
			case SV_RING_RESIST_POIS:
			case SV_RING_FREE_ACTION:
			case SV_RING_SEE_INVIS:
				return (TRUE);
		}
	}
	else if (o_ptr->tval == TV_AMULET)
	{
		switch(o_ptr->sval)
		{
			case SV_AMULET_RESIST_ACID:
			case SV_AMULET_RESIST:
			case SV_AMULET_REGEN:
			case SV_AMULET_RESIST_LIGHTNING:
				return (TRUE);
		}
	}
	/* Staffs only if not aware */
	/* because staff psuedo can indicate egos also */
	else if ((o_ptr->tval == TV_STAFF) && (!object_aware_p(o_ptr)))
	{
		switch(o_ptr->sval)
		{
			case SV_STAFF_DESTRUCTION:
			case SV_STAFF_POWER:
			case SV_STAFF_HOLINESS:
			case SV_STAFF_BANISHMENT:
            case SV_STAFF_SPEED:
            case SV_STAFF_MANAFREE:
				return (TRUE);
		}
	}
	return (FALSE);
}
#endif
/*
 * Return a "feeling" (or NULL) about an item.  Method 1 (Heavy).
 */
int value_check_aux1(const object_type *o_ptr)
{
	/* Artifacts */
	if (artifact_p(o_ptr))
	{
		/* Cursed/Broken */
		if (cursed_p(o_ptr) || broken_p(o_ptr)) return (INSCRIP_TERRIBLE);

		/* Normal */
		return (INSCRIP_SPECIAL);
	}

#ifdef EFG
	/* EFGchange new pseudo level SPLENDID */
	/* now that all standarts are splendid, should there even be INSCRIP_SPECIAL? */
	if (object_splendid_p(o_ptr))
		return (INSCRIP_SPLENDID);
	/* EFGchange pseudo jewelry */
	if (excellent_jewelry_p(o_ptr)) return (INSCRIP_EXCELLENT);
#endif
	/* Ego-Items */
	if (ego_item_p(o_ptr))
	{
		/* Cursed/Broken */
		if (cursed_p(o_ptr) || broken_p(o_ptr)) return (INSCRIP_WORTHLESS);

		/* Normal */
		return (INSCRIP_EXCELLENT);
	}

	/* Cursed items */
	if (cursed_p(o_ptr)) return (INSCRIP_CURSED);

	/* Broken items */
	if (broken_p(o_ptr)) return (INSCRIP_BROKEN);

	/* Good "armor" bonus */
	if (o_ptr->to_a > 2) return (INSCRIP_GOOD);

	/* Good "weapon" bonus */
	if (o_ptr->to_h + o_ptr->to_d > 2) return (INSCRIP_GOOD);
	
	/* Default to "average" */
	return (INSCRIP_AVERAGE);
}


/*
 * Return a "feeling" (or NULL) about an item.  Method 2 (Light).
 */
static int value_check_aux2(const object_type *o_ptr)
{
	/* Cursed items (all of them) */
	if (cursed_p(o_ptr)) return (INSCRIP_CURSED);

	/* Broken items (all of them) */
	if (broken_p(o_ptr)) return (INSCRIP_BROKEN);

#ifdef EFG
	/* EFGchange new pseudo level SPLENDID */
	if (object_splendid_p(o_ptr)) return (INSCRIP_SPLENDID);
	if (excellent_jewelry_p(o_ptr)) return (INSCRIP_GOOD);
#endif
	/* Artifacts -- except cursed/broken ones */
	if (artifact_p(o_ptr)) return (INSCRIP_GOOD);

	/* Ego-Items -- except cursed/broken ones */
	if (ego_item_p(o_ptr)) return (INSCRIP_GOOD);

	/* Good armor bonus */
	if (o_ptr->to_a > 2) return (INSCRIP_GOOD);

	/* Good weapon bonuses */
	if (o_ptr->to_h + o_ptr->to_d > 2) return (INSCRIP_GOOD);

	/* No feeling */
	return (0);
}



/*
 * Sense the inventory
 */
static void sense_inventory(void)
{
	int i;
#ifdef EFG
	bool sensedany;
	int phase;
#endif

	int plev = p_ptr->lev;

	bool heavy = ((cp_ptr->flags & CF_PSEUDO_ID_HEAVY) ? TRUE : FALSE);

	int feel;

	object_type *o_ptr;
	object_kind *k_ptr;
	char o_name[80];


	/*** Check for "sensing" ***/

	/* No sensing when confused */
	if (p_ptr->timed[TMD_CONFUSED]) return;

	if (cp_ptr->flags & CF_PSEUDO_ID_IMPROV)
	{
		if (0 != rand_int(cp_ptr->sense_base / (plev * plev + cp_ptr->sense_div)))
			return;
	}
	else
	{
		if (0 != rand_int(cp_ptr->sense_base / (plev + cp_ptr->sense_div)))
			return;
	}


	/*** Sense everything ***/

#ifdef EFG
	/* EFGchange know non-pseudoed wielded items average when anything pseudos */
	sensedany = FALSE;
	phase;
	for (phase = 0; phase <= 1; phase++) for (i = phase*INVEN_WIELD; i < INVEN_TOTAL; i++)
#else
	/* Check everything */
	for (i = 0; i < INVEN_TOTAL; i++)
#endif
	{
		bool okay = FALSE;

		o_ptr = &inventory[i];

		/* Skip empty slots */
		if (!o_ptr->k_idx) continue;

		/* Valid "tval" codes */
		switch (o_ptr->tval)
		{
			case TV_SHOT:
			case TV_ARROW:
			case TV_BOLT:
			case TV_BOW:
			case TV_DIGGING:
			case TV_HAFTED:
			case TV_POLEARM:
			case TV_SWORD:
			case TV_BOOTS:
			case TV_GLOVES:
			case TV_HELM:
			case TV_CROWN:
			case TV_SHIELD:
			case TV_CLOAK:
			case TV_SOFT_ARMOR:
			case TV_HARD_ARMOR:
			case TV_DRAG_ARMOR:
                 /* staffs and tusks can get egos so they should pseudo */
			case TV_STAFF:
			case TV_SKELETON:
#ifdef EFG
			/* EFGchange allow pseudo on jewelry */
			case TV_LITE:
			case TV_AMULET:
			case TV_RING:
#endif
			{
				okay = TRUE;
				break;
			}
		}

		/* Skip non-sense machines */
		if (!okay) continue;

#ifdef EFG
		/* EFGchange know non-pseudoed wielded items average when anything pseudos */
		/* anything that is fully pseudoed should have IDENT_SENSE */
		/* semi pseudo like INDESTRUCTIBLE or SPLENDID does not */
#else
		/* It's already been pseudo-ID'd */
		if (o_ptr->pseudo &&
		    o_ptr->pseudo != INSCRIP_INDESTRUCTIBLE) continue;
#endif

		/* It has already been sensed, do not sense it again */
		if (o_ptr->ident & (IDENT_SENSE)) continue;

		/* It is known, no information needed */
		if (object_known_p(o_ptr)) continue;

		k_ptr = &k_info[o_ptr->k_idx];

		/* always sense if weight is different */
		if (o_ptr->weight < k_ptr->weight) /* okay */;
		/* Occasional failure on inventory items */
		else if ((i < INVEN_WIELD) && (0 != rand_int(5))) continue;

		/* Indestructible objects are either excellent or terrible */
		if (o_ptr->pseudo == INSCRIP_INDESTRUCTIBLE)
			heavy = TRUE;

#ifdef EFG
		/* EFGchange know non-pseudoed wielded items average when anything pseudos */
		/* The second pass with phase=1 we know everything non-avg */
		/* has pseudoed, and the player should know anything else  */
		/* currently wielded is avg, if anything pseudoed at all.  */
		if (phase && sensedany)
			heavy = TRUE;
#endif
		/* Check for a feeling */
		feel = (heavy ? value_check_aux1(o_ptr) : value_check_aux2(o_ptr));

		/* Skip non-feelings */
		if (!feel) continue;

		/* Stop everything */
		if (disturb_minor) disturb(0, 0);
#ifdef EFG
		/* EFGchange know non-pseudoed wielded items average when anything pseudos */
		sensedany = TRUE;
#endif

		/* Get an object description */
		object_desc(o_name, sizeof(o_name), o_ptr, FALSE, 0);

		/* Message (equipment) */
		sound(MSG_PSEUDOID);

		if (i >= INVEN_WIELD)
		{
#ifdef EFG
			/* EFGchange know non-pseudoed wielded items average when anything pseudos */
			msg_format("You %s the %s (%c) you are %s %s %s...",
				phase ? "realize" : "feel",
#else
			msg_format("You feel the %s (%c) you are %s %s %s...",
#endif
			           o_name, index_to_label(i), describe_use(i),
			           ((o_ptr->number == 1) ? "is" : "are"),
			           inscrip_text[feel - INSCRIP_NULL]);
		}

		/* Message (inventory) */
		else
		{
			msg_format("You feel the %s (%c) in your pack %s %s...",
			           o_name, index_to_label(i),
			           ((o_ptr->number == 1) ? "is" : "are"),
			           inscrip_text[feel - INSCRIP_NULL]);
		}

		/* Sense the object */
		o_ptr->pseudo = feel;

		/* The object has been "sensed" */
		o_ptr->ident |= (IDENT_SENSE);

		if (o_ptr->name1)
		{
			/* EFGchange remove need for identify wrto preserving artifacts */
			object_known(o_ptr);
			msg_print("You recognize a legendary item!");

			/* artifact powers shouldn't be hidden */
			o_ptr->ident |= (IDENT_MENTAL);
		}
		/* EFGchange make stuff that pseudos as average be as if identified */
		/* ??? This does not pick up jewelry -- good or bad? */
		if ((feel == INSCRIP_AVERAGE) && (object_aware_p(o_ptr)))
			/* so you don't have to remember how many times enchanted "avg" longbow */
			object_known(o_ptr);

		if (o_ptr->tval == TV_STAFF)
		{
           /* no squelching magic staffs by quality */
        }
		/* Set squelch flag as appropriate */
		else if (i < INVEN_WIELD)
			p_ptr->notice |= PN_SQUELCH;


		/* Combine / Reorder the pack (later) */
		p_ptr->notice |= (PN_COMBINE | PN_REORDER);

		/* Window stuff */
		p_ptr->window |= (PW_INVEN | PW_EQUIP);
	}
}



/*
 * Regenerate hit points
 */
static void regenhp(int percent)
{
	s32b new_chp, new_chp_frac;
	int old_chp;
	int maxhp = p_ptr->mhp;

	/* HP regeneration is stopped */
	if (p_ptr->stopregen) return;

	/* False life spell raises maxhp */
	if (p_ptr->timed[TMD_FALSE_LIFE]) maxhp += 2 * (p_ptr->lev + 10);

	/* Save the old hitpoints */
	old_chp = p_ptr->chp;

	/* Extract the new hitpoints */
	new_chp = ((long)maxhp) * percent + PY_REGEN_HPBASE;
	p_ptr->chp += (s16b)(new_chp >> 16);   /* div 65536 */

	/* check for overflow */
	if ((p_ptr->chp < 0) && (old_chp > 0)) p_ptr->chp = MAX_SHORT;
	new_chp_frac = (new_chp & 0xFFFF) + p_ptr->chp_frac;	/* mod 65536 */
	if (new_chp_frac >= 0x10000L)
	{
		p_ptr->chp_frac = (u16b)(new_chp_frac - 0x10000L);
		p_ptr->chp++;
	}
	else
	{
		p_ptr->chp_frac = (u16b)new_chp_frac;
	}

	/* Fully healed */
	if (p_ptr->chp >= maxhp)
	{
		p_ptr->chp = maxhp;
		p_ptr->chp_frac = 0;
	}

	/* Notice changes */
	if (old_chp != p_ptr->chp)
	{
		/* Redraw */
		p_ptr->redraw |= (PR_HP);

		/* Window stuff */
		p_ptr->window |= (PW_PLAYER_0 | PW_PLAYER_1);
	}
}


/*
 * Regenerate mana points
 */
static void regenmana(int percent)
{
	s32b new_mana, new_mana_frac;
	int old_csp;

	old_csp = p_ptr->csp;
	new_mana = ((long)p_ptr->msp) * percent + PY_REGEN_MNBASE;
	p_ptr->csp += (s16b)(new_mana >> 16);	/* div 65536 */
	/* check for overflow */
	if ((p_ptr->csp < 0) && (old_csp > 0))
	{
		p_ptr->csp = MAX_SHORT;
	}
	new_mana_frac = (new_mana & 0xFFFF) + p_ptr->csp_frac;	/* mod 65536 */
	if (new_mana_frac >= 0x10000L)
	{
		p_ptr->csp_frac = (u16b)(new_mana_frac - 0x10000L);
		p_ptr->csp++;
	}
	else
	{
		p_ptr->csp_frac = (u16b)new_mana_frac;
	}

	/* Must set frac to zero even if equal */
	if (p_ptr->csp >= p_ptr->msp)
	{
		p_ptr->csp = p_ptr->msp;
		p_ptr->csp_frac = 0;
	}

	/* Redraw mana */
	if (old_csp != p_ptr->csp)
	{
		/* Redraw */
		p_ptr->redraw |= (PR_MANA);

		/* Window stuff */
		p_ptr->window |= (PW_PLAYER_0 | PW_PLAYER_1);
	}
}






/*
 * Regenerate the monsters (once per 100 game turns)
 *
 * XXX XXX XXX Should probably be done during monster turns.
 */
static void regen_monsters(void)
{
	int i, frac;

	/* Regenerate everyone */
	for (i = 1; i < mon_max; i++)
	{
		/* Check the i'th monster */
		monster_type *m_ptr = &mon_list[i];
		monster_race *r_ptr = &r_info[m_ptr->r_idx];

		/* Skip dead monsters */
		if (!m_ptr->r_idx) continue;
		
		/* DJA: make HELPER monsters leave (only if not too far from the PC) */
		/* because they can occationally be placed on themed levels */
		/* without being summoned. */
		if ((r_ptr->flags3 & (RF3_HELPER)) && (randint(100) < 20 - goodluck/3) &&
			(m_ptr->cdis <= MAX_RANGE))
		{
           if (m_ptr->ml) msg_print("Your helper leaves.");
           delete_monster_idx(i);
        }

		/* Allow regeneration (if needed) */
		if (m_ptr->hp < m_ptr->maxhp)
		{
			/* Hack -- Base regeneration */
			frac = m_ptr->maxhp / 100;

			/* Hack -- Minimal regeneration rate */
			if (!frac) frac = 1;

			/* Hack -- Some monsters regenerate quickly */
			if (r_ptr->flags2 & (RF2_REGENERATE)) frac *= 2;

			/* Hack -- Regenerate */
			m_ptr->hp += frac;

			/* Do not over-regenerate */
			if (m_ptr->hp > m_ptr->maxhp) m_ptr->hp = m_ptr->maxhp;

			/* Redraw (later) if needed */
			if (p_ptr->health_who == i) p_ptr->redraw |= (PR_HEALTH);
		}
	}
}


/*
 * If player has inscribed the object with "!!", let him know when it's
 * recharged. -LM-
 * Also inform player when first item of a stack has recharged. -HK-
 */
static void recharged_notice(const object_type *o_ptr, bool all)
{
	char o_name[120];

	cptr s;

	/* No inscription */
	if (!o_ptr->note) return;

	/* Find a '!' */
	s = strchr(quark_str(o_ptr->note), '!');

	/* Process notification request */
	while (s)
	{
		/* Find another '!' */
		if (s[1] == '!')
		{
			/* Describe (briefly) */
			object_desc(o_name, sizeof(o_name), o_ptr, FALSE, 0);

			/* Disturb the player */
			if (disturb_minor) disturb(0, 0);

			/* Notify the player */
			if (o_ptr->number > 1)
			{
				if (all) msg_format("Your %s have recharged.", o_name);
				else msg_format("One of your %s has recharged.", o_name);
			}

			/* Artifacts */
			else if (o_ptr->name1)
			{
				msg_format("The %s has recharged.", o_name);
			}

			/* Single, non-artifact items */
			else msg_format("Your %s has recharged.", o_name);

			/* Done */
			return;
		}

		/* Keep looking for '!'s */
		s = strchr(s + 1, '!');
	}
}


/*
 * Recharge activatable objects in the player's equipment
 * and rods in the inventory and on the ground.
 */
static void recharge_objects(void)
{
	int i;
	int charged = 0;
	bool chargeokay = TRUE;

	object_type *o_ptr;
	object_kind *k_ptr;


	/* Process equipment */
	for (i = INVEN_WIELD; i < INVEN_TOTAL; i++)
	{
		u32b f1, f2, f3, f4;

		/* Get the object */
		o_ptr = &inventory[i];

		/* Skip non-objects */
		if (!o_ptr->k_idx) continue;

		/* get flags */
		object_flags(o_ptr, &f1, &f2, &f3, &f4);

		/* is it an activatable light source? */
		if ((o_ptr->tval == TV_LITE) && (f3 & TR3_ACTIVATE)) chargeokay = TRUE;
		/* other light sources use timeout for fuel */
		else if (o_ptr->tval == TV_LITE) chargeokay = FALSE;

		/* Recharge activatable objects */
		if (o_ptr->timeout > 0 && (chargeokay))
		{
			/* Recharge */
			o_ptr->timeout--;

			/* Notice changes */
			if (!(o_ptr->timeout)) charged++;

			/* Message if item is recharged, if inscribed with "!!" */
			if (!(o_ptr->timeout)) recharged_notice(o_ptr, TRUE);
		}

		/* handle constant activation */
		if ((i == INVEN_WIELD) && (f2 & (TR2_CONSTANTA)))
		{
			/* Currently only on magic staffs, but that could possibly change */
			if (o_ptr->tval == TV_STAFF)
			{
				if (o_ptr->sval == SV_STAFF_ZAPPING)
				{
					if (p_ptr->timed[TMD_ZAPPING] < 11) (void)set_timed(TMD_ZAPPING, 15);
				}
				else if (o_ptr->sval == SV_STAFF_SPEED)
				{
					if (p_ptr->timed[TMD_FAST] < 11) (void)set_timed(TMD_FAST, 15);
				}
				else if (o_ptr->sval == SV_STAFF_LITE)
				{
					if (p_ptr->timed[TMD_MINDLIGHT] < 11) (void)set_timed(TMD_MINDLIGHT, 15);
				}
#if blah
				else if (o_ptr->sval == SV_STAFF_SLOWNESS)
				{
					if (p_ptr->timed[TMD_SLOW] < 11) (void)set_timed(TMD_SLOW, 15);
				} /* (staff of slowness removed) */
#endif
			}
		}
	}

	/* Notice changes */
	if (charged)
	{
		/* Window stuff */
		p_ptr->window |= (PW_EQUIP);
	}

	charged = 0;

	/* Recharge rods */
	for (i = 0; i < INVEN_PACK; i++)
	{
		o_ptr = &inventory[i];
		k_ptr = &k_info[o_ptr->k_idx];

		/* Skip non-objects */
		if (!o_ptr->k_idx) continue;

		/* Examine all charging rods */
		if ((o_ptr->tval == TV_ROD) && (o_ptr->timeout))
		{
			/* Determine how many rods are charging */
			int temp = (o_ptr->timeout + (k_ptr->pval - 1)) / k_ptr->pval;

			if (temp > o_ptr->number) temp = o_ptr->number;

			/* Decrease timeout by that number */
			o_ptr->timeout -= temp;

			/* Boundary control */
			if (o_ptr->timeout < 0) o_ptr->timeout = 0;

			/* Update if any rods are recharged */
			if (temp > (o_ptr->timeout + (k_ptr->pval - 1)) / k_ptr->pval)
			{
				/* Update window */
				charged++;

				/* Message if whole stack is recharged, if inscribed with "!!" */
				if (!(o_ptr->timeout)) recharged_notice(o_ptr, TRUE);
				/* Message if first in a stack is recharged, if inscribed with "!!" -HK- */
				else if (temp == o_ptr->number) recharged_notice(o_ptr, FALSE);
			}
		}
	}

	/* Notice changes */
	if (charged)
	{
		/* Combine pack */
		p_ptr->notice |= (PN_COMBINE);

		/* Window stuff */
		p_ptr->window |= (PW_INVEN);
	}


	/*** Process Objects ***/

	/* Process objects */
	for (i = 1; i < o_max; i++)
	{
		/* Get the object */
		o_ptr = &o_list[i];

		/* Skip dead objects */
		if (!o_ptr->k_idx) continue;

		/* Recharge rods on the ground */
		if ((o_ptr->tval == TV_ROD) && (o_ptr->timeout))
		{
			/* Charge it */
			o_ptr->timeout -= o_ptr->number;

			/* Boundary control */
			if (o_ptr->timeout < 0) o_ptr->timeout = 0;
		}
	}
}


static void play_ambient_sound(void)
{
	/* Town sound */
	if (p_ptr->depth == 0) 
	{
		/* Hack - is it daytime or nighttime? */
		if (turn % (10L * TOWN_DAWN) < TOWN_DAWN / 2)
		{
			/* It's day. */
			sound(MSG_AMBIENT_DAY);
		} 
		else 
		{
			/* It's night. */
			sound(MSG_AMBIENT_NITE);
		}
		
	}

	/* Dungeon level 1-20 */
	else if (p_ptr->depth <= 20) 
	{
		sound(MSG_AMBIENT_DNG1);
	}

	/* Dungeon level 21-40 */
	else if (p_ptr->depth <= 40) 
	{
		sound(MSG_AMBIENT_DNG2);
	}

	/* Dungeon level 41-60 */
	else if (p_ptr->depth <= 60) 
	{
		sound(MSG_AMBIENT_DNG3);
	}

	/* Dungeon level 61-80 */
	else if (p_ptr->depth <= 80) 
	{
		sound(MSG_AMBIENT_DNG4);
	}

	/* Dungeon level 80- */
	else  
	{
		sound(MSG_AMBIENT_DNG5);
	}
}

/*
 * Helper for process_world -- decrement p_ptr->timed[] fields.
 */
static void decrease_timeouts(void)
{
	int adjust = (adj_con_fix[p_ptr->stat_ind[A_CON]] + 1);
	int i;
	object_type *o_ptr;

	/* golems have no constitution score (and are immune to cutting) */
	/* (no goodluck or badluck except from timed effects) */
	if (p_ptr->prace == 16)
	{
		adjust = randint(4);
		if (badluck) adjust -= 1;
		if (goodluck) adjust += 1;
	}
	
	/* timed weapon blessing */
	o_ptr = &inventory[INVEN_WIELD];
    if ((o_ptr->blessed > 1) && ((!p_ptr->resting) || (randint(100) < 33)))
    {
	   if (o_ptr->blessed == 2)
       {
	      /* get the object name */
          char o_name[80];
	      object_desc(o_name, sizeof(o_name), o_ptr, FALSE, 1);

          msg_format("The blessing on your %s has faded", o_name);
       }
	   o_ptr->blessed -= 1;
	   /* o_ptr->blessed remains at 1 to nullify badweap flag */
    }
    /* find vault spell wears off after awhile if not used */
    if ((p_ptr->find_vault) && (!p_ptr->resting) && (randint(35+goodluck) == 2))
        p_ptr->find_vault -= 1;

	/* Turn big rocks into rubble when MIGHTY_HURL expires */
	if (p_ptr->timed[TMD_MIGHTY_HURL] == 1)
	{
		/* Scan all objects */
		for (i = 1; i < o_max; i++)
		{
			object_type *o_ptr = &o_list[i];
			int y, x;

			/* Skip held objects */
			if (o_ptr->held_m_idx) continue;

			/* is it a big rock? */
			if ((o_ptr->tval == TV_SKELETON) && (o_ptr->sval == SV_BIG_ROCK))
			{
				/* Location */
				y = o_ptr->iy;
				x = o_ptr->ix;

				/* Create rubble (if there isn't already rubble there) */
				if (cave_feat[y][x] == FEAT_OPEN_PIT) cave_set_feat(y, x, FEAT_FLOOR);
			    else if (cave_feat[y][x] != FEAT_RUBBLE) cave_set_feat(y, x, FEAT_RUBBLE);

				/* Update the visuals */
				p_ptr->update |= (PU_UPDATE_VIEW | PU_MONSTERS);
			}
		}
	}

	/* clear p_ptr->held_m_idx when you pull free of the hold */
	if (p_ptr->timed[TMD_BEAR_HOLD] == 1) p_ptr->held_m_idx = 0;

	/* reset maxhp when FALSE_LIFE ends */
	if (p_ptr->timed[TMD_FALSE_LIFE] == 1)
	{
		if (p_ptr->chp >= p_ptr->mhp)
		{
			p_ptr->chp = p_ptr->mhp;
			p_ptr->chp_frac = 0;

			/* redraw */
			p_ptr->redraw |= (PR_HP);
			p_ptr->window |= (PW_PLAYER_0 | PW_PLAYER_1);
		}
	}

	/* Decrement all effects that can be done simply */
	for (i = 0; i < TMD_MAX; i++)
	{
		int decr = 1;
		if (!p_ptr->timed[i])
			continue;

		switch (i)
		{
			case TMD_CUT:
			{
				/* Hack -- check for truly "mortal" wound */
				decr = (p_ptr->timed[i] > 1000) ? 0 : adjust;
				break;
			}

			case TMD_STUN:
			{
				decr = adjust;
				break;
			}
			case TMD_POISONED:
			{
				if (p_ptr->weakresist_pois) decr = adjust + 1;
				else decr = adjust;
				break;
			}
		}
		/* Decrement the effect */
		dec_timed(i, decr);
	}

	/* Sustain speed sustains positive adjusted speed (but not hasting) */
	if ((p_ptr->timed[TMD_SUST_SPEED]) && (p_ptr->timed[TMD_ADJUST]))
	{
		if (p_ptr->spadjust > 0) (void)inc_timed(TMD_ADJUST, 3);
	}

	return;
}


/*
 * Handle certain things once every 10 game turns
 */
static void process_world(void)
{
	int i;
	int corch;

	int regen_amount;

	object_type *o_ptr;
	int maxhp = p_ptr->mhp;
	if (p_ptr->timed[TMD_FALSE_LIFE]) maxhp += 2 * (p_ptr->lev + 10);

	/* Every 10 game turns */
	if (turn % 10) return;

	/*** Check the Time and Load ***/

	/* Play an ambient sound at regular intervals. */
	if (!(turn % ((10L * TOWN_DAWN) / 4)))
	{
		play_ambient_sound();
	}

	/*** Handle the "town" (stores and sunshine) ***/

	/* While in town */
	if (!p_ptr->depth)
	{
		/* Hack -- Daybreak/Nighfall in town */
		if (!(turn % ((10L * TOWN_DAWN) / 2)))
		{
			bool dawn;

			/* Check for dawn */
			dawn = (!(turn % (10L * TOWN_DAWN)));

			/* Day breaks */
			if (dawn)
				msg_print("The sun has risen.");

			/* Night falls */
			else
				msg_print("The sun has fallen.");

			/* Illuminate */
			town_illuminate(dawn);
		}
	}


	/* While in the dungeon */
	else
	{
		/*** Update the Stores ***/

		/* Update the stores once a day (while in dungeon) */
		if (!(turn % (10L * STORE_TURNS)))
		{
			int n;

			/* Message */
			if (cheat_xtra) msg_print("Updating Shops...");

			/* Maintain each shop (except home) */
			for (n = 0; n < MAX_STORES; n++)
			{
				/* Skip the home */
				if (n == STORE_HOME) continue;

				/* Maintain */
				store_maint(n);
			}

			/* Sometimes, shuffle the shop-keepers */
			if (rand_int(STORE_SHUFFLE) == 0)
			{
				/* Message */
				if (cheat_xtra) msg_print("Shuffling a Shopkeeper...");

				/* Pick a random shop (except home) */
				while (1)
				{
					n = rand_int(MAX_STORES);
					if (n != STORE_HOME) break;
				}

				/* Shuffle it */
				store_shuffle(n);
			}

			/* Message */
			if (cheat_xtra) msg_print("Done.");
		}
	}

	/*** Process the monsters ***/

	/* Check for creature generation */
	if (rand_int(MAX_M_ALLOC_CHANCE) == 0)
	{
		/* Make a new monster */
		(void)alloc_monster(MAX_SIGHT + 5, FALSE);
	}

	/* Hack -- Check for creature regeneration */
	if (!(turn % 100)) regen_monsters();


	/*** Damage over Time ***/
	
	/* DAJ: effects of silver poison */
	if (p_ptr->silver > PY_SILVER_HEALTHY)
	{
        if ((p_ptr->silver >= PY_SILVER_LEVELONE) && (p_ptr->silver < PY_SILVER_LEVELTWO))
        {
           	int rare = randint(1000 - p_ptr->silver);
      	    if (rare == 1)
      	    {
                     int die = randint(100);
                     if (die < 56) msg_print("The silver poison affects your mind.");

                     if (die < 20) (p_ptr->stat_cur[A_WIS] = p_ptr->stat_cur[A_WIS] -1);
                     else if (die < 30) (p_ptr->stat_cur[A_INT] = p_ptr->stat_cur[A_INT] -1);
                     else if (die < 34) (p_ptr->stat_cur[A_CHR] = p_ptr->stat_cur[A_CHR] -1);
                     else if (die < 45) (void)inc_timed(TMD_AMNESIA, rand_int(3) + 4);
                     else if (die < 56) (void)inc_timed(TMD_IMAGE, rand_int(4) + 4);
                     else take_hit(1, "silver poison");
            }
            
        }
        if ((p_ptr->silver >= PY_SILVER_LEVELTWO) && (p_ptr->silver < PY_SILVER_VERYBAD))
        {
           	int rare = randint(1000 - p_ptr->silver);
      	    if (rare < 4)
      	    {
                     int die = randint(100);
                     if (die < 90) msg_print("The silver poison affects your mind.");

                     if (die < 30) (p_ptr->stat_cur[A_WIS] = p_ptr->stat_cur[A_WIS] -1);
                     else if (die < 45) (p_ptr->stat_cur[A_INT] = p_ptr->stat_cur[A_INT] -1);
                     else if (die < 49) (p_ptr->stat_cur[A_CHR] = p_ptr->stat_cur[A_CHR] -1);
                     else if (die < 54) (void)inc_timed(TMD_CONFUSED, rand_int(4) + 4);
                     else if (die < 72) (void)inc_timed(TMD_AMNESIA, rand_int(6) + 4);
                     else if (die < 90) (void)inc_timed(TMD_IMAGE, rand_int(6) + 4);
                     else take_hit(randint(p_ptr->silver-4), "silver poison");
            }
            
        }
        if (p_ptr->silver >= PY_SILVER_VERYBAD)
        {
           	int notsorare = randint(100 - badluck);
      	    if (notsorare < 33)
      	    {
               p_ptr->stat_cur[A_WIS] = p_ptr->stat_cur[A_WIS] -1;
               p_ptr->stat_cur[A_INT] = p_ptr->stat_cur[A_INT] -1;
               if (notsorare < 10) (p_ptr->stat_cur[A_CHR] = p_ptr->stat_cur[A_CHR] -1);
		       take_hit(66 + randint(p_ptr->silver*6), "corruption from silver poison");
               if (notsorare < 16) (void)inc_timed(TMD_CONFUSED, randint(4) + 3);
               if (notsorare > 16) (void)inc_timed(TMD_AMNESIA, randint(6) + 6);
               if (notsorare < 25) (void)inc_timed(TMD_IMAGE, randint(6) + 4);
        	   msg_print("Your mind is corrupted by silver poison!");
            }
        } 
                       
    }

	/* DAJ: effects of slime */
	if (p_ptr->slime > PY_SLIME_HEALTHY)
	{
        if ((p_ptr->slime >= PY_SLIME_LEVELONE) && (p_ptr->slime < PY_SLIME_LEVELTWO))
        {
           	int rare = randint(1000 - (p_ptr->slime/2));
      	    if (rare == 1)
      	    {
                     int die = randint(100);
                     if (die < 55) msg_print("The slime oozes into your body.");

                     if (die < 25) (p_ptr->stat_cur[A_CON] = p_ptr->stat_cur[A_CON] -1);
                     else if (die < 35) (p_ptr->stat_cur[A_DEX] = p_ptr->stat_cur[A_DEX] -1);
                     else if (die < 40) (p_ptr->stat_cur[A_CHR] = p_ptr->stat_cur[A_CHR] -1);
                     else if (die < 55) take_hit(1, "slime");
                     else ;
            }
            
        }
        if ((p_ptr->slime >= PY_SLIME_LEVELTWO) && (p_ptr->slime < PY_SLIME_VERYBAD))
        {
           	int rare = randint(1000 - (p_ptr->slime/2));
      	    if (rare < 4)
      	    {
                     int die = randint(100);
                     msg_print("The slime oozes into your body.");

                     if (die < 30) (p_ptr->stat_cur[A_CON] = p_ptr->stat_cur[A_CON] -1);
                     else if (die < 45) (p_ptr->stat_cur[A_DEX] = p_ptr->stat_cur[A_DEX] -1);
                     else if (die < 50) (p_ptr->stat_cur[A_CHR] = p_ptr->stat_cur[A_CHR] -1);
                     else if (die < 70) (void)inc_timed(TMD_SLOW, rand_int(4) + 5);
                     else if (die < 90) (void)inc_timed(TMD_STUN, rand_int(4) + 5);
                     else take_hit(randint(p_ptr->slime-30), "slime");
            }
            
        }
        if (p_ptr->slime >= PY_SLIME_VERYBAD)
        {
           	int notsorare = randint(100 - badluck);
      	    if (notsorare < 33)
      	    {
               p_ptr->stat_cur[A_CON] = p_ptr->stat_cur[A_CON] -1;
               p_ptr->stat_cur[A_DEX] = p_ptr->stat_cur[A_DEX] -1;
               p_ptr->stat_cur[A_CHR] = p_ptr->stat_cur[A_CHR] -1;
		       take_hit(33 + randint(p_ptr->slime * 4), "slime");
               if (notsorare < 16) (void)inc_timed(TMD_SLOW, randint(4) + 6);
               if (notsorare > 16) (void)inc_timed(TMD_STUN, randint(6) + 6);
        	   msg_print("Slime is taking over your body!");
            }
        } 
                       
    }
    
    /* corruption from the One Ring */
    corch = 5000; /* no effect at 5000 */
    if (p_ptr->corrupt > 0) corch = randint(6000 - badluck);
    if (corch < 4)
    {
       p_ptr->corrupt += 1;
       if ((corch < 3) && (randint(100) > 20 + goodluck)) p_ptr->luck -= 1;
       /* corruption limit */
       if (p_ptr->corrupt > 50) p_ptr->corrupt = 50;
    }
    else if ((corch < (p_ptr->corrupt / 2) - (goodluck / 2) - 1) && (p_ptr->corrupt >= 20))
    {
       if ((badluck > 15) && (p_ptr->max_depth > 69) && (randint(100) < 33))
       {
          /* may happen at any depth, as long as max depth is > 69 */
          if (summon_specific(p_ptr->py, p_ptr->px, 70, SUMMON_WRAITH))
          {
             msg_print("A Nazgul has sought you out.");
          }
          else summon_specific(p_ptr->py, p_ptr->px, p_ptr->max_depth/2, SUMMON_DEMON);
       }
       else
       {
          summon_specific(p_ptr->py, p_ptr->px, p_ptr->max_depth-6, SUMMON_DEMON);
          msg_print("Demons are attracted to the evil power you hold.");
       }
    }

    /* good magic items can remove corruption */
	/* (as long as you're not still wearing the corrupting item) */
    if ((goodweap) && (!badweap) && (p_ptr->corrupt > 0) && (rand_int(999) < goodweap + 1))
    {
       p_ptr->corrupt -= 1;
    }
    
    /* attraction of demons by black magic or bad luck */    
    if (((p_ptr->timed[TMD_WITCH]) || (badluck > 18)) && (randint(666) == 1))
    {
		int sy, sx;
		if (cp_ptr->spell_book == TV_DARK_BOOK)
       {
          msg_print("Demons are attracted to your black magic.");
       }
       else if (badluck > 18)
       {
          msg_print("Demons are attracted to those who are very unlucky.");
       }
       else
       {
          msg_print("Demons are attracted to the nether power you summoned.");
       }
	   /* get a nearby (about 20 spaces away) location to place the summoned demon */
	   /* (just place it next to the PC about 40% of the time) */
	   if ((!get_nearby(p_ptr->py, p_ptr->px, &sy, &sx, 3)) || 
		   (rand_int(100) < 40 + badluck))
	   {
		   sy = p_ptr->py;
		   sx = p_ptr->px;
	   }
	   if (p_ptr->max_depth < 3) /* no summon */;
       else if ((randint(100) < (p_ptr->lev + p_ptr->max_depth)/3) && 
		   (p_ptr->lev + p_ptr->max_depth >= 46)) do_call_help(560);
	   else if (!p_ptr->depth) summon_specific(sy, sx, 1, SUMMON_DEMON);
       else if (p_ptr->max_depth > 7) summon_specific(sy, sx, p_ptr->max_depth-5, SUMMON_DEMON);
	   else summon_specific(sy, sx, 2, SUMMON_DEMON);
    }
    
    /* neutral class wielding both good object(s) and bad object(s) */
    if ((magicmod > 19) && (randint(999) == 1))
    {
       msg_print("Some of your equipped items are in conflict with each other.");
       if (p_ptr->resist_confu) (void)inc_timed(TMD_CONFUSED, 1 + randint(2 + (badluck/5)));
       else (void)inc_timed(TMD_CONFUSED, 2 + randint(4 + (badluck/3)));
    }
    /* aligned class wielding both good object(s) and bad object(s) */
    if (((magicmod == 10) || (magicmod == 8)) && (randint(1200) == 1))
    {
       msg_print("Some of your equipped items are in conflict with each other.");
       if (p_ptr->resist_confu) (void)inc_timed(TMD_CONFUSED, 1 + randint(2));
       else (void)inc_timed(TMD_CONFUSED, 2 + randint(4));      
       if (randint(100) < 60) (void)inc_timed(TMD_STUN, 1 + badweap);
    }
    if (((magicmod == 1) || (magicmod == 3)) && (randint(1200) == 1))
    {
       msg_print("Some of your equipped items are in conflict with each other.");
       if (p_ptr->resist_confu) (void)inc_timed(TMD_CONFUSED, 1 + randint(2 + (badluck/5)));
       else (void)inc_timed(TMD_CONFUSED, 2 + randint(3 + (badluck/3)));
       if (randint(100) < 60) (void)inc_timed(TMD_STUN, 1 + goodweap);
    }

	/* may start to drown if paralyzed or stunned in water */
	if (cave_feat[p_ptr->py][p_ptr->px] == FEAT_WATER)
	{
		bool waterhold = FALSE;
		/* check if being held by a water monster */
		if (p_ptr->timed[TMD_BEAR_HOLD])
		{
			monster_type *m_ptr = &mon_list[p_ptr->held_m_idx];
			monster_race *r_ptr = &r_info[m_ptr->r_idx];
			if ((r_ptr->flags7 & (RF7_WATER_HIDE)) || (r_ptr->flags7 & (RF7_WATER_ONLY)))
				waterhold = TRUE;
		}
		
		/* paralyzed, stunned, or held underwater */
		if (((p_ptr->timed[TMD_PARALYZED]) && (randint(100) < 22)) ||
			((waterhold) && (randint(100) < 21)) ||
			((p_ptr->timed[TMD_STUN] > 9) && 
			(randint(100) < 5 + p_ptr->timed[TMD_STUN]/5)))
		{
			int drown;
			if (p_ptr->mhp > 100) drown = damroll(2, 20);
			else if (p_ptr->mhp < 10) drown = damroll(2, 2);
			else drown = damroll(2, p_ptr->mhp/5);
			/* make sure the player knows he can die by drowning */
			if (!p_ptr->warned)
			{
				/* warn the player */
				if (p_ptr->timed[TMD_PARALYZED])
				{
					msg_print("Being paralyzed, you can't keep your head above water: ");
				}
				else if (waterhold) msg_print("Your head is being held underwater: ");
				else msg_print("You are too stunned to keep your head above water: ");
				p_ptr->warned = 1;
				/* be easy on the PC the first time */
				if (drown > p_ptr->chp) drown = p_ptr->chp - randint(3);
				else if (drown > 18) drown -= randint(2);
			}
			msg_print("You start to drown!");
			take_hit(drown, "drowning");
		}
	}
    
	/* Take damage from poison */
	if (p_ptr->timed[TMD_POISONED])
	{
		/* Take damage */
		take_hit(1, "poison");
	}

	/* Take damage from cuts */
	if (p_ptr->timed[TMD_CUT])
	{
		/* Mortal wound or Deep Gash */
		if (p_ptr->timed[TMD_CUT] > 200)
			i = 3;

		/* Severe cut */
		else if (p_ptr->timed[TMD_CUT] > 100)
			i = 2;

		/* Other cuts */
		else
			i = 1;

		/* Take damage */
		take_hit(i, "a fatal wound");
	}


	/*** Check the Food, and Regenerate ***/

	/* Digest normally */
	if (p_ptr->food < PY_FOOD_MAX)
	{
		/* Every 100 game turns */
		if (!(turn % 100))
		{
			/* Basic digestion rate based on speed */
			i = extract_energy[p_ptr->pspeed] * 2;

			/* Regeneration takes more food (was 30) */
			if (p_ptr->regenerate) i += 28;

			/* digest faster when hasted */
			if (p_ptr->timed[TMD_SUST_SPEED]) i += 20;

			/* Slow digestion takes less food */
			if (p_ptr->slow_digest) i -= 11;

			/* Minimal digestion */
			if (i < 1) i = 1;

			/* Digest some food */
			(void)set_food(p_ptr->food - i);
		}
	}

	/* Digest quickly when gorged */
	else
	{
		/* Digest a lot of food */
		(void)set_food(p_ptr->food - 100);
	}

	/* Starve to death (slowly) */
	if (p_ptr->food < PY_FOOD_STARVE)
	{
		/* Calculate damage */
		i = (PY_FOOD_STARVE - p_ptr->food) / 10;

		/* Take damage */
		take_hit(i, "starvation");
	}

	/* Default regeneration */
	regen_amount = PY_REGEN_NORMAL;

	/* Getting Weak */
	if (p_ptr->food < PY_FOOD_WEAK)
	{
		/* Lower regeneration */
		if (p_ptr->food < PY_FOOD_STARVE)
		{
			regen_amount = 0;
		}
		else if (p_ptr->food < PY_FOOD_FAINT)
		{
			regen_amount = PY_REGEN_FAINT;
		}
		else
		{
			regen_amount = PY_REGEN_WEAK;
		}

		/* Getting Faint */
		if (p_ptr->food < PY_FOOD_FAINT)
		{
			/* Faint occasionally */
			if (!p_ptr->timed[TMD_PARALYZED] && (rand_int(100) < 10))
			{
				/* Message */
				msg_print("You faint from the lack of food.");
				disturb(1, 0);

				/* Hack -- faint (bypass free action) */
				(void)inc_timed(TMD_PARALYZED, 1 + rand_int(5));
			}
		}
	}

	/* Regeneration ability */
	if (p_ptr->regenerate)
	{
		regen_amount = regen_amount * 2;
	}

	/* Searching or Resting */
	if (p_ptr->searching || p_ptr->resting)
	{
		regen_amount = regen_amount * 2;
	}

	/* Regenerate the mana */
	if (p_ptr->csp < p_ptr->msp)
	{
		regenmana(regen_amount);
	}

	/* Various things interfere with healing */
	if (p_ptr->timed[TMD_PARALYZED]) regen_amount /= 3;
	if (p_ptr->timed[TMD_POISONED]) regen_amount = 0;
	if (p_ptr->timed[TMD_STUN]) regen_amount /= 4;
	if (p_ptr->timed[TMD_CUT]) regen_amount = 0;
	if (p_ptr->timed[TMD_BECOME_LICH]) regen_amount /= 2;
	if (p_ptr->stopregen) regen_amount = 0;

	/* Regenerate Hit Points if needed */
	if (p_ptr->chp < maxhp)
	{
		regenhp(regen_amount);
	}


	/*** Timeout Various Things ***/

	decrease_timeouts();



	/*** Process Light ***/

	/* Check for light being wielded */
	o_ptr = &inventory[INVEN_LITE];

	/* Burn some fuel in the current lite */
	if (o_ptr->tval == TV_LITE)
	{
		u32b f1, f2, f3, f4;
		bool burn_fuel = TRUE;

		/* Get the object flags */
		object_flags(o_ptr, &f1, &f2, &f3, &f4);

		/* Turn off the wanton burning of light during the day in the town */
		if (!p_ptr->depth && ((turn % (10L * TOWN_DAWN)) < ((10L * TOWN_DAWN) / 2)))
			burn_fuel = FALSE;

		/* If the light has the NO_FUEL flag, well... */
		if (f3 & TR3_NO_FUEL)
		    burn_fuel = FALSE;

		/* Use some fuel */
		if (burn_fuel && o_ptr->timeout > 0)
		{
			/* Decrease life-span */
			o_ptr->timeout--;

			/* Hack -- notice interesting fuel steps */
			if ((o_ptr->timeout < 100) || (!(o_ptr->timeout % 100)))
			{
				/* Window stuff */
				p_ptr->window |= (PW_EQUIP);
			}

			/* Hack -- Special treatment when blind */
			if (p_ptr->timed[TMD_BLIND])
			{
				/* Hack -- save some light for later */
				if (o_ptr->timeout == 0) o_ptr->timeout++;
			}

			/* The light is now out */
			else if (o_ptr->timeout == 0)
			{
				disturb(0, 0);
				msg_print("Your light has gone out!");
			}

			/* The light is getting dim */
			else if ((o_ptr->timeout < 100) && (!(o_ptr->timeout % 10)))
			{
				if (disturb_minor) disturb(0, 0);
				msg_print("Your light is growing faint.");
			}
		}
	}

	/* Calculate torch radius */
	p_ptr->update |= (PU_TORCH);


	/*** Process Inventory ***/

	/* Recharge activatable objects and rods */
	recharge_objects();

	/* Feel the inventory */
	sense_inventory();


	/*** Involuntary Movement ***/

	/* Mega-Hack -- Random teleportation XXX XXX XXX */
	if ((p_ptr->teleport) && (rand_int(100) < 1))
	{
        bool controlled = FALSE;
        /* controlled teleport (not easily controlled) */
		if (p_ptr->telecontrol)
		{
            if (control_tport(0, 101)) controlled = TRUE;

            if (!controlled) msg_print("You fail to control the teleportation.");
        }

        /* check for controlled teleport before teleporting randomly */
        if (!controlled)
        {
            /* Teleport player randomly */
		    teleport_player(40);
        }
	}

	/* Delayed Word-of-Recall */
	if (p_ptr->word_recall)
	{
		/* Count down towards recall */
		p_ptr->word_recall--;

		/* recall twice as fast if desperate to escape */
		if ((p_ptr->timed[TMD_TERROR]) && (p_ptr->depth)) p_ptr->word_recall--;

		/* Activate the recall */
		if (!p_ptr->word_recall)
		{
			/* Disturbing! */
			disturb(0, 0);

			/* Sound */
			sound(MSG_TPLEVEL);

			/* Determine the level */
			if (p_ptr->depth)
			{
				msg_print("You feel yourself yanked upwards!");

				/* New depth */
				p_ptr->depth = 0;

				/* Leaving */
				p_ptr->leaving = TRUE;
			}
			else
			{
				msg_print("You feel yourself yanked downwards!");

				/* New depth */
				p_ptr->depth = p_ptr->max_depth;
				if (p_ptr->depth < 1) p_ptr->depth = 1;

				/* Leaving */
				p_ptr->leaving = TRUE;
			}
		}
	}
}





/*
 * Hack -- helper function for "process_player()"
 *
 * Check for changes in the "monster memory"
 */
static void process_player_aux(void)
{
	int i;
	bool changed = FALSE;

	static int old_monster_race_idx = 0;

	static u32b	old_flags1 = 0L;
	static u32b	old_flags2 = 0L;
	static u32b	old_flags3 = 0L;
	static u32b	old_flags4 = 0L;
	static u32b	old_flags5 = 0L;
	static u32b	old_flags6 = 0L;

	static byte old_blows[MONSTER_BLOW_MAX];

	static byte	old_cast_innate = 0;
	static byte	old_cast_spell = 0;


	/* Tracking a monster */
	if (p_ptr->monster_race_idx)
	{
		/* Get the monster lore */
		monster_lore *l_ptr = &l_list[p_ptr->monster_race_idx];

		for (i = 0; i < MONSTER_BLOW_MAX; i++)
		{
			if (old_blows[i] != l_ptr->blows[i])
			{
				changed = TRUE;
				break;
			}
		}
		
		/* Check for change of any kind */
		if (changed ||
		    (old_monster_race_idx != p_ptr->monster_race_idx) ||
		    (old_flags1 != l_ptr->flags1) ||
		    (old_flags2 != l_ptr->flags2) ||
		    (old_flags3 != l_ptr->flags3) ||
		    (old_flags4 != l_ptr->flags4) ||
		    (old_flags5 != l_ptr->flags5) ||
		    (old_flags6 != l_ptr->flags6) ||
		    (old_cast_innate != l_ptr->cast_innate) ||
		    (old_cast_spell != l_ptr->cast_spell))
		{
			/* Memorize old race */
			old_monster_race_idx = p_ptr->monster_race_idx;

			/* Memorize flags */
			old_flags1 = l_ptr->flags1;
			old_flags2 = l_ptr->flags2;
			old_flags3 = l_ptr->flags3;
			old_flags4 = l_ptr->flags4;
			old_flags5 = l_ptr->flags5;
			old_flags6 = l_ptr->flags6;

			/* Memorize blows */
			for (i = 0; i < MONSTER_BLOW_MAX; i++)
				old_blows[i] = l_ptr->blows[i];

			/* Memorize castings */
			old_cast_innate = l_ptr->cast_innate;
			old_cast_spell = l_ptr->cast_spell;

			/* Window stuff */
			p_ptr->window |= (PW_MONSTER);

			/* Window stuff */
			window_stuff();
		}
	}
}


/*
 * Process the player
 *
 * Notice the annoying code to handle "pack overflow", which
 * must come first just in case somebody manages to corrupt
 * the savefiles by clever use of menu commands or something.
 *
 * Notice the annoying code to handle "monster memory" changes,
 * which allows us to avoid having to update the window flags
 * every time we change any internal monster memory field, and
 * also reduces the number of times that the recall window must
 * be redrawn.
 *
 * Note that the code to check for user abort during repeated commands
 * and running and resting can be disabled entirely with an option, and
 * even if not disabled, it will only check during every 128th game turn
 * while resting, for efficiency.
 */
static void process_player(void)
{
	int i;
	int item;
	int maxhp = p_ptr->mhp;
	if (p_ptr->timed[TMD_FALSE_LIFE]) maxhp += 2 * (p_ptr->lev + 10);

	/*** Check for interrupts ***/

	/* Complete resting */
	if (p_ptr->resting < 0)
	{
		/* Basic resting */
#ifdef EFG
		/* EFGchange rest '|' to match rest '&' */
		if (p_ptr->resting == REST_EITHER)
		{
			/* Stop resting */
			if ((p_ptr->chp == maxhp) || (p_ptr->csp == p_ptr->msp))
			{
				disturb(0, 0);
			}
		}
		else if (p_ptr->resting == REST_BOTH)
#else
		if (p_ptr->resting == -1)
#endif
		{
			/* Stop resting */
			/* (don't wait for hp regeneration if it's not going to happen) */
			if (((p_ptr->chp == maxhp) || (p_ptr->stopregen)) &&
			    (p_ptr->csp == p_ptr->msp))
			{
				disturb(0, 0);
			}
		}

		/* Complete resting */
#ifdef EFG
		/* EFGchange rest '|' to match rest '&' */
		else if (p_ptr->resting == REST_FULL)
#else
		else if (p_ptr->resting == -2)
#endif
		{
			bool minor = TRUE;
			/* disturb minor is a wierd option to use for this.. */
			if (disturb_minor) /* minor timed effects (only slightly bad) */
			{
                minor = FALSE;
                if (!p_ptr->timed[TMD_FRENZY] && !p_ptr->timed[TMD_CURSE] &&
                   !p_ptr->timed[TMD_SUST_SPEED] && !p_ptr->timed[TMD_2ND_THOUGHT] &&
                   !p_ptr->timed[TMD_STONESKIN] && !p_ptr->timed[TMD_AMNESIA])
                   minor = TRUE;
            }
            
            /* Stop resting */
			if (((p_ptr->chp == maxhp) || (p_ptr->stopregen)) &&
			    (p_ptr->csp == p_ptr->msp) && (minor) &&
			    !p_ptr->timed[TMD_BLIND] && !p_ptr->timed[TMD_CONFUSED] &&
			    !p_ptr->timed[TMD_POISONED] && !p_ptr->timed[TMD_AFRAID] &&
			    !p_ptr->timed[TMD_STUN] && !p_ptr->timed[TMD_CUT] && !p_ptr->timed[TMD_TERROR] &&
			    !p_ptr->timed[TMD_SLOW] && !p_ptr->timed[TMD_PARALYZED] &&
			    !p_ptr->timed[TMD_IMAGE] && !p_ptr->word_recall && !p_ptr->timed[TMD_CHARM])
			{
				disturb(0, 0);
			}
		}
	}

	/* Check for "player abort" */
	if (p_ptr->running ||
	    p_ptr->command_rep ||
	    (p_ptr->resting && !(turn & 0x7F)))
	{
		/* Do not wait */
		inkey_scan = TRUE;

		/* Check for a key */
		if (inkey())
		{
			/* Flush input */
			flush();

			/* Disturb */
			disturb(0, 0);

			/* Hack -- Show a Message */
			msg_print("Cancelled.");
		}
	}


	/*** Handle actual user input ***/

	/* Repeat until energy is reduced */
	do
	{
		/* Notice stuff (if needed) */
		if (p_ptr->notice) notice_stuff();

		/* Update stuff (if needed) */
		if (p_ptr->update) update_stuff();

		/* Redraw stuff (if needed) */
		if (p_ptr->redraw) redraw_stuff();

		/* Redraw stuff (if needed) */
		if (p_ptr->window) window_stuff();


		/* Place the cursor on the player */
		move_cursor_relative(p_ptr->py, p_ptr->px);

		/* Refresh (optional) */
		Term_fresh();


		/* Hack -- Pack Overflow */
		/* if (inventory[INVEN_PACK].k_idx) */
		for (item = INVEN_PACK;
			item >= INVEN_PACK - p_ptr->pack_size_reduce; item--)
		{
			char o_name[80];

			object_type *o_ptr;

			/* Get the slot to be dropped */
			o_ptr = &inventory[item];

			/* Ignore empty objects */
			if (!o_ptr->k_idx) continue;

			/* Disturbing */
			disturb(0, 0);

			/* Warning */
			msg_print("Your pack overflows!");

			/* Describe */
			object_desc(o_name, sizeof(o_name), o_ptr, TRUE, 3);

			/* Message */
			msg_format("You drop %s (%c).", o_name, index_to_label(item));

			/* Drop it (carefully) near the player */
			drop_near(o_ptr, 0, p_ptr->py, p_ptr->px);

			/* Modify, Describe, Optimize */
			inven_item_increase(item, -255);
			inven_item_describe(item);
			inven_item_optimize(item);
            
			/* Notice stuff (if needed) */
			if (p_ptr->notice) notice_stuff();

			/* Update stuff (if needed) */
			if (p_ptr->update) update_stuff();

			/* Redraw stuff (if needed) */
			if (p_ptr->redraw) redraw_stuff();

			/* Window stuff (if needed) */
			if (p_ptr->window) window_stuff();
		}


		/* Hack -- cancel "lurking browse mode" */
		if (!p_ptr->command_new) p_ptr->command_see = FALSE;


		/* Assume free turn */
		p_ptr->energy_use = 0;


		/* Paralyzed or Knocked Out */
		if ((p_ptr->timed[TMD_PARALYZED]) || (p_ptr->timed[TMD_STUN] >= 100))
		{
			/* Take a turn */
			p_ptr->energy_use = 100;
		}

		/* Picking up objects */
		else if (p_ptr->notice & (PN_PICKUP))
		{
			/* Recursively call the pickup function, use energy */
			p_ptr->energy_use = py_pickup(0) * 10;
			p_ptr->notice &= ~(PN_PICKUP);
		}

		/* Resting */
		else if (p_ptr->resting)
		{
			/* Timed rest */
			if (p_ptr->resting > 0)
			{
				/* Reduce rest count */
				p_ptr->resting--;

				/* Redraw the state */
				p_ptr->redraw |= (PR_STATE);
			}

			/* Take a turn */
			p_ptr->energy_use = 100;
		}

		/* Running */
		else if (p_ptr->running)
		{
			/* Take a step */
			run_step(0);
		}

		/* Repeated command */
		else if (p_ptr->command_rep)
		{
			/* Hack -- Assume messages were seen */
			msg_flag = FALSE;

			/* Clear the top line */
			prt("", 0, 0);

			/* Process the command */
			process_command(TRUE);

			/* Count this execution */
			if (p_ptr->command_rep)
			{
				/* Count this execution */
				p_ptr->command_rep--;

				/* Redraw the state */
				p_ptr->redraw |= (PR_STATE);
			}
		}

		/* Normal command */
		else
		{
			/* Check monster recall */
			process_player_aux();

			/* Place the cursor on the player */
			move_cursor_relative(p_ptr->py, p_ptr->px);

			/* Get and process a command */
			process_command(FALSE);
		}


		/*** Clean up ***/

		/* Significant */
		if (p_ptr->energy_use)
		{
			/* Use some energy */
			p_ptr->energy -= p_ptr->energy_use;

			/* Hack -- constant hallucination */
			if (p_ptr->timed[TMD_IMAGE])
			{
				p_ptr->redraw |= (PR_MAP);
				p_ptr->window |= (PW_MAP);
			}

			/* Shimmer monsters if needed */
			if (shimmer_monsters)
			{
				/* Clear the flag */
				shimmer_monsters = FALSE;

				/* Shimmer multi-hued monsters */
				for (i = 1; i < mon_max; i++)
				{
					monster_type *m_ptr;
					monster_race *r_ptr;

					/* Get the monster */
					m_ptr = &mon_list[i];

					/* Skip dead monsters */
					if (!m_ptr->r_idx) continue;

					/* Get the monster race */
					r_ptr = &r_info[m_ptr->r_idx];

					/* Skip non-multi-hued monsters */
					if (!(r_ptr->flags1 & (RF1_ATTR_MULTI))) continue;

					/* Reset the flag */
					shimmer_monsters = TRUE;

					/* Redraw regardless */
					lite_spot(m_ptr->fy, m_ptr->fx);
				}
			}

			/* Repair "nice" flags */
			if (repair_mflag_nice)
			{
				/* Clear flag */
				repair_mflag_nice = FALSE;

				/* Process monsters */
				for (i = 1; i < mon_max; i++)
				{
					monster_type *m_ptr;

					/* Get the monster */
					m_ptr = &mon_list[i];

					/* Skip dead monsters */
					/* if (!m_ptr->r_idx) continue; */

					/* Clear "nice" flag */
					m_ptr->mflag &= ~(MFLAG_NICE);
				}
			}

			/* Repair "mark" flags */
			if (repair_mflag_mark)
			{
				/* Reset the flag */
				repair_mflag_mark = FALSE;

				/* Process the monsters */
				for (i = 1; i < mon_max; i++)
				{
					monster_type *m_ptr;

					/* Get the monster */
					m_ptr = &mon_list[i];

					/* Skip dead monsters */
					/* if (!m_ptr->r_idx) continue; */

					/* Repair "mark" flag */
					if (m_ptr->mflag & (MFLAG_MARK))
					{
						/* Skip "show" monsters */
						if (m_ptr->mflag & (MFLAG_SHOW))
						{
							/* Repair "mark" flag */
							repair_mflag_mark = TRUE;

							/* Skip */
							continue;
						}

						/* Forget flag */
						m_ptr->mflag &= ~(MFLAG_MARK);

						/* Update the monster */
						update_mon(i, FALSE);
					}
				}
			}
		}

		/* Repair "show" flags */
		if (repair_mflag_show)
		{
			/* Reset the flag */
			repair_mflag_show = FALSE;

			/* Process the monsters */
			for (i = 1; i < mon_max; i++)
			{
				monster_type *m_ptr;

				/* Get the monster */
				m_ptr = &mon_list[i];

				/* Skip dead monsters */
				/* if (!m_ptr->r_idx) continue; */

				/* Clear "show" flag */
				m_ptr->mflag &= ~(MFLAG_SHOW);
			}
		}
	}

	while (!p_ptr->energy_use && !p_ptr->leaving);
}



/*
 * Interact with the current dungeon level.
 *
 * This function will not exit until the level is completed,
 * the user dies, or the game is terminated.
 */
static void dungeon(void)
{
	monster_type *m_ptr;
	int i;

	int py = p_ptr->py;
	int px = p_ptr->px;


	/* Hack -- enforce illegal panel */
	Term->offset_y = DUNGEON_HGT;
	Term->offset_x = DUNGEON_WID;


	/* Not leaving */
	p_ptr->leaving = FALSE;


	/* Reset the "command" vars */
	p_ptr->command_cmd = 0;
	p_ptr->command_new = 0;
	p_ptr->command_rep = 0;
	p_ptr->command_arg = 0;
	p_ptr->command_dir = 0;


	/* Cancel the target */
	target_set_monster(0);

	/* Cancel the health bar */
	health_track(0);


	/* Reset shimmer flags */
	shimmer_monsters = TRUE;
	shimmer_objects = TRUE;

	/* Reset repair flags */
	repair_mflag_nice = TRUE;
	repair_mflag_show = TRUE;
	repair_mflag_mark = TRUE;


	/* Disturb */
	disturb(1, 0);


	/* Track maximum player level */
	if (p_ptr->max_lev < p_ptr->lev)
	{
		p_ptr->max_lev = p_ptr->lev;
	}


	/* Track maximum dungeon level */
	if (p_ptr->max_depth < p_ptr->depth)
	{
		p_ptr->max_depth = p_ptr->depth;
	}


	/* No stairs down from Quest */
	if (is_quest(p_ptr->depth))
	{
		p_ptr->create_down_stair = FALSE;
	}

	/* No stairs from town or if not allowed */
	if (!p_ptr->depth || adult_no_stairs)
	{
		p_ptr->create_down_stair = p_ptr->create_up_stair = FALSE;
	}

	/* Make a staircase */
	if (p_ptr->create_down_stair || p_ptr->create_up_stair)
	{
		/* Place a staircase */
		if (cave_valid_bold(py, px))
		{
			/* XXX XXX XXX */
			delete_object(py, px);

			/* Make stairs */
			if (p_ptr->create_down_stair)
			{
				cave_set_feat(py, px, FEAT_MORE);
			}
			else
			{
				cave_set_feat(py, px, FEAT_LESS);
			}

			/* Mark the stairs as known */
			cave_info[py][px] |= (CAVE_MARK);
		}

		/* Cancel the stair request */
		p_ptr->create_down_stair = p_ptr->create_up_stair = FALSE;
	}

	/* Choose panel */
	verify_panel();

	/* Flush messages */
	message_flush();


	/* Hack -- Increase "xtra" depth */
	character_xtra++;


	/* Clear */
	Term_clear();


	/* Update stuff */
	p_ptr->update |= (PU_BONUS | PU_HP | PU_MANA | PU_SPELLS);

	/* Calculate torch radius */
	p_ptr->update |= (PU_TORCH);

	/* Update stuff */
	update_stuff();


	/* Fully update the visuals (and monster distances) */
	p_ptr->update |= (PU_FORGET_VIEW | PU_UPDATE_VIEW | PU_DISTANCE);

	/* Fully update the flow */
	p_ptr->update |= (PU_FORGET_FLOW | PU_UPDATE_FLOW);

	/* Redraw dungeon */
	p_ptr->redraw |= (PR_BASIC | PR_EXTRA | PR_MAP | PR_EQUIPPY);

	/* Window stuff */
	p_ptr->window |= (PW_INVEN | PW_EQUIP | PW_PLAYER_0 | PW_PLAYER_1);

	/* Window stuff */
	p_ptr->window |= (PW_MONSTER | PW_MONLIST | PW_OBJLIST);

	/* Window stuff */
	p_ptr->window |= (PW_OVERHEAD | PW_MAP);

	/* Update stuff */
	update_stuff();

	/* Redraw stuff */
	redraw_stuff();

	/* Redraw stuff */
	window_stuff();


	/* Hack -- Decrease "xtra" depth */
	character_xtra--;


	/* Update stuff */
	p_ptr->update |= (PU_BONUS | PU_HP | PU_MANA | PU_SPELLS);

	/* Combine / Reorder the pack */
	p_ptr->notice |= (PN_COMBINE | PN_REORDER);

	/* Notice stuff */
	notice_stuff();

	/* Update stuff */
	update_stuff();

	/* Redraw stuff */
	redraw_stuff();

	/* Window stuff */
	window_stuff();

	/* Refresh */
	Term_fresh();


	/* Handle delayed death */
	if (p_ptr->is_dead) return;


	/* Announce (or repeat) the feeling */
	if (p_ptr->depth) do_cmd_feeling();


	/*** Process this dungeon level ***/

	/* Reset the monster generation level */
	monster_level = p_ptr->depth;

	/* Reset the object generation level */
	object_level = p_ptr->depth;
	
	/* Reset the group roaming destinations */
	roamgroup1 = 0;
	roamgroup2 = 0;
	roamgroup3 = 0;
	roamgroup4 = 0;
	roamgroup5 = 0;
	roamgroup6 = 0;
	roamgroup7 = 0;
	roamgroup8 = 0;

	/* Main loop */
	while (TRUE)
	{
		/* Hack -- Compact the monster list occasionally */
		if (mon_cnt + 32 > z_info->m_max) compact_monsters(64);

		/* Hack -- Compress the monster list occasionally */
		if (mon_cnt + 32 < mon_max) compact_monsters(0);


		/* Hack -- Compact the object list occasionally */
		if (o_cnt + 32 > z_info->o_max) compact_objects(64);

		/* Hack -- Compress the object list occasionally */
		if (o_cnt + 32 < o_max) compact_objects(0);


		/* Can the player move? */
		while ((p_ptr->energy >= 100) && !p_ptr->leaving)
		{
			/* process monster with even more energy first */
			process_monsters((byte)(p_ptr->energy + 1));

			/* if still alive */
			if (!p_ptr->leaving)
			{
				/* Process the player */
				process_player();
			}
		}

		/* Notice stuff */
		if (p_ptr->notice) notice_stuff();

		/* Update stuff */
		if (p_ptr->update) update_stuff();

		/* Redraw stuff */
		if (p_ptr->redraw) redraw_stuff();

		/* Redraw stuff */
		if (p_ptr->window) window_stuff();

		/* Hack -- Hilite the player */
		move_cursor_relative(p_ptr->py, p_ptr->px);

		/* Handle "leaving" */
		if (p_ptr->leaving) break;


		/* Process all of the monsters */
		process_monsters(100);

		/* Notice stuff */
		if (p_ptr->notice) notice_stuff();

		/* Update stuff */
		if (p_ptr->update) update_stuff();

		/* Redraw stuff */
		if (p_ptr->redraw) redraw_stuff();

		/* Redraw stuff */
		if (p_ptr->window) window_stuff();

		/* Hack -- Hilite the player */
		move_cursor_relative(p_ptr->py, p_ptr->px);

		/* Handle "leaving" */
		if (p_ptr->leaving) break;


		/* Process the world */
		process_world();

		/* Notice stuff */
		if (p_ptr->notice) notice_stuff();

		/* Update stuff */
		if (p_ptr->update) update_stuff();

		/* Redraw stuff */
		if (p_ptr->redraw) redraw_stuff();

		/* Window stuff */
		if (p_ptr->window) window_stuff();

		/* Hack -- Hilite the player */
		move_cursor_relative(p_ptr->py, p_ptr->px);

		/* Handle "leaving" */
		if (p_ptr->leaving) break;


		/*** Apply energy ***/

		/* Give the player some energy */
		p_ptr->energy += extract_energy[p_ptr->pspeed];

		/* Give energy to all monsters */
		for (i = mon_max - 1; i >= 1; i--)
		{
			/* Access the monster */
			monster_race *r_ptr;
			m_ptr = &mon_list[i];
			r_ptr = &r_info[m_ptr->r_idx];

			/* Ignore "dead" monsters */
			if (!m_ptr->r_idx) continue;

			/* Give this monster some energy */
			m_ptr->energy += extract_energy[m_ptr->mspeed];

			/* WATER_ONLY monsters out of water try to get back to water quickly */
			if ((r_ptr->flags7 & (RF7_WATER_ONLY)) &&
				(!(cave_feat[m_ptr->fy][m_ptr->fx] == FEAT_WATER)))
			{
				m_ptr->energy += 10;
			}
		}

		/* Count game turns */
		turn++;
	}
}



/*
 * Process some user pref files
 */
static void process_some_user_pref_files(void)
{
	char buf[1024];


	/* Process the "user.prf" file */
	(void)process_pref_file("user.prf");

	/* Get the "PLAYER.prf" filename */
	(void)strnfmt(buf, sizeof(buf), "%s.prf", op_ptr->base_name);

	/* Process the "PLAYER.prf" file */
	(void)process_pref_file(buf);
}


/*
 * Actually play a game.
 *
 * This function is called from a variety of entry points, since both
 * the standard "main.c" file, as well as several platform-specific
 * "main-xxx.c" files, call this function to start a new game with a
 * new savefile, start a new game with an existing savefile, or resume
 * a saved game with an existing savefile.
 *
 * If the "new_game" parameter is true, and the savefile contains a
 * living character, then that character will be killed, so that the
 * player may start a new game with that savefile.  This is only used
 * by the "-n" option in "main.c".
 *
 * If the savefile does not exist, cannot be loaded, or contains a dead
 * (non-wizard-mode) character, then a new game will be started.
 *
 * Several platforms (Windows, Macintosh, Amiga) start brand new games
 * with "savefile" and "op_ptr->base_name" both empty, and initialize
 * them later based on the player name.  To prevent weirdness, we must
 * initialize "op_ptr->base_name" to "PLAYER" if it is empty.
 *
 * Note that we load the RNG state from savefiles (2.8.0 or later) and
 * so we only initialize it if we were unable to load it.  The loading
 * code marks successful loading of the RNG state using the "Rand_quick"
 * flag, which is a hack, but which optimizes loading of savefiles.
 */
void play_game(bool new_game)
{
	bool character_loaded;
	bool reusing_savefile;
	int maxhp;

	/* Hack -- Increase "icky" depth */
	character_icky++;


	/* Verify main term */
	if (!term_screen)
	{
		quit("main window does not exist");
	}

	/* Make sure main term is active */
	Term_activate(term_screen);

	/* Verify minimum size */
	if ((Term->hgt < 24) || (Term->wid < 80))
	{
		quit("main window is too small");
	}

	/* Hack -- Turn off the cursor */
	(void)Term_set_cursor(FALSE);

	/* Attempt to load */
	if (!load_player(&character_loaded, &reusing_savefile))
	{
		/* Oops */
		quit("broken savefile");
	}
	msg_format("just loaded");

	/* Nothing loaded */
	if (!character_loaded)
	{
		/* Make new player */
		new_game = TRUE;

		/* The dungeon is not ready */
		character_dungeon = FALSE;

		/* XXX This is the place to add automatic character
		   numbering (i.e. Rocky IV, V, etc.) Probably. */
	}

	/* Hack -- Default base_name */
	if (!op_ptr->base_name[0])
	{
		my_strcpy(op_ptr->base_name, "PLAYER", sizeof(op_ptr->base_name));
	}

	/* Init RNG */
	if (Rand_quick)
	{
		u32b seed;

		/* Basic seed */
		seed = (time(NULL));

#ifdef SET_UID

		/* Mutate the seed on Unix machines */
		seed = ((seed >> 3) * (getpid() << 1));

#endif

		/* Use the complex RNG */
		Rand_quick = FALSE;

		/* Seed the "complex" RNG */
		Rand_state_init(seed);
	}

	/* Roll new character */
	if (new_game)
	{
		/* The dungeon is not ready */
		character_dungeon = FALSE;

		/* Start in town */
		p_ptr->depth = 0;

		/* Hack -- seed for flavors */
		seed_flavor = rand_int(0x10000000);

		/* Hack -- seed for town layout */
		seed_town = rand_int(0x10000000);

		/* Hack -- seed for random artifacts */
		seed_randart = rand_int(0x10000000);

		/* Roll up a new character */
		player_birth();

		/* Randomize the artifacts */
		if (adult_randarts)
			do_randart(seed_randart, TRUE);

		/* Hack -- enter the world */
		turn = 1;
	}

	/* Normal machine (process player name) */
	if (savefile[0])
	{
		process_player_name(FALSE);
	}

	/* Weird machine (process player name, pick savefile name) */
	else
	{
		process_player_name(TRUE);
	}
        
	/* Check if we're overwriting a savefile */
	while (!reusing_savefile && my_fexists(savefile))
	{
		/* Ask for confirmation */
		bool overwrite = get_check("Continuing will overwrite an existing savefile.  Overwrite? ");
         
		if (overwrite)
		{
			break;
		}                        
		else
		{
			get_name(TRUE);
		}                        
	}

	/* Flash a message */
	prt("Please wait...", 0, 0);

	/* Flush the message */
	Term_fresh();


	/* Flavor the objects */
	flavor_init();

	/* Reset visuals */
	reset_visuals(TRUE);


	/* Window stuff */
	p_ptr->window |= (PW_INVEN | PW_EQUIP | PW_PLAYER_0 | PW_PLAYER_1);
	p_ptr->window |= (PW_MONSTER | PW_MESSAGE);
	window_stuff();


	/* Process some user pref files */
	process_some_user_pref_files();


	/* React to changes */
	Term_xtra(TERM_XTRA_REACT, 0);


	/* Generate a dungeon level if needed */
	if (!character_dungeon) generate_cave();


	/* Character is now "complete" */
	character_generated = TRUE;


	/* Hack -- Decrease "icky" depth */
	character_icky--;


	/* Start playing */
	p_ptr->playing = TRUE;

	/* Hack -- Enforce "delayed death" */
	if (p_ptr->chp < 0) p_ptr->is_dead = TRUE;

	/* Process */
	while (TRUE)
	{
		/* Play ambient sound on change of level. */
		play_ambient_sound();

		/* Process the level */
		dungeon();

		/* Notice stuff */
		if (p_ptr->notice) notice_stuff();

		/* Update stuff */
		if (p_ptr->update) update_stuff();

		/* Redraw stuff */
		if (p_ptr->redraw) redraw_stuff();

		/* Window stuff */
		if (p_ptr->window) window_stuff();


		/* Cancel the target */
		target_set_monster(0);

		/* Cancel the health bar */
		health_track(0);


		/* Forget the view */
		forget_view();


		/* Handle "quit and save" */
		if (!p_ptr->playing && !p_ptr->is_dead) break;


		/* Erase the old cave */
		wipe_o_list();
		wipe_mon_list();


		/* XXX XXX XXX */
		message_flush();

		/* Accidental Death */
		if (p_ptr->playing && p_ptr->is_dead)
		{
			maxhp = p_ptr->mhp;

			/* Mega-Hack -- Allow player to cheat death */
			if ((p_ptr->wizard || cheat_live) && !get_confirm("Die? "))
			{
				/* Mark social class, reset age, if needed */
				if (p_ptr->sc) p_ptr->sc = p_ptr->age = 0;

				/* Increase age (used to count wizmode deaths) */
				p_ptr->age++;

				/* Mark savefile */
				p_ptr->noscore |= NOSCORE_WIZARD;

				/* Message */
				msg_print("You invoke wizard mode and cheat death.");
				message_flush();

				/* Cheat death */
				p_ptr->is_dead = FALSE;

				/* Restore hit points */
				p_ptr->chp = maxhp;
				p_ptr->chp_frac = 0;

				/* Restore spell points */
				p_ptr->csp = p_ptr->msp;
				p_ptr->csp_frac = 0;

				/* Hack -- Healing */
				(void)clear_timed(TMD_BLIND);
				(void)clear_timed(TMD_CONFUSED);
				(void)clear_timed(TMD_POISONED);
				(void)clear_timed(TMD_AFRAID);
				(void)clear_timed(TMD_CHARM);
				(void)clear_timed(TMD_FRENZY);
				(void)clear_timed(TMD_PARALYZED);
				(void)clear_timed(TMD_IMAGE);
				(void)clear_timed(TMD_STUN);
				(void)clear_timed(TMD_CUT);
				p_ptr->silver = 0;
				p_ptr->slime = 0;

				/* Hack -- Prevent starvation */
				if (p_ptr->food <= PY_FOOD_ALERT) (void)set_food(PY_FOOD_MAX - 1);
				else if (p_ptr->food < PY_FOOD_FULL + 2500)  (void)set_food(p_ptr->food + 2500);

				/* Hack -- cancel recall */
				if (p_ptr->word_recall)
				{
					/* Message */
					msg_print("A tension leaves the air around you...");
					message_flush();

					/* Hack -- Prevent recall */
					p_ptr->word_recall = 0;
				}

				/* Note cause of death XXX XXX XXX */
				my_strcpy(p_ptr->died_from, "Cheating death", sizeof(p_ptr->died_from));

				/* New depth */
				p_ptr->depth = 0;

				/* Leaving */
				p_ptr->leaving = TRUE;
			}
		}

		/* Handle "death" */
		if (p_ptr->is_dead) break;

		/* Make a new level */
		generate_cave();

		/* autosave whenever a new level is generated */
		save_player();
	}

	/* Close stuff */
	close_game();
}
