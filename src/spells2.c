/* File: spells2.c */

/* Purpose: Spell code (part 2) */

/*
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */

#include "angband.h"

#define WEIRD_LUCK      12
#define BIAS_LUCK       20
/*
 * Bias luck needs to be higher than weird luck,
 * since it is usually tested several times...
 */

void summon_dragon_riders();


/*
 * Grow trees
 */
void grow_trees(int rad)
{
        int a,i,j;

        for(a=0;a<rad*rad+11;a++){
                i=(Rand_mod((rad*2)+1)-rad+Rand_mod((rad*2)+1)-rad)/2;
                j=(Rand_mod((rad*2)+1)-rad+Rand_mod((rad*2)+1)-rad)/2;
                if (in_bounds(py + i, px + j) && (cave_clean_bold(py+i, px+j)))
                {
                        cave_set_feat(py+i, px+j, FEAT_TREES);
                }
        }
}

/*
 * Increase players hit points, notice effects
 */
bool hp_player(int num)
{
	/* Healing needed */
	if (p_ptr->chp < p_ptr->mhp)
	{
		/* Gain hitpoints */
		p_ptr->chp += num;

		/* Enforce maximum */
		if (p_ptr->chp >= p_ptr->mhp)
		{
			p_ptr->chp = p_ptr->mhp;
			p_ptr->chp_frac = 0;
		}

		/* Redraw */
		p_ptr->redraw |= (PR_HP);

		/* Window stuff */
		p_ptr->window |= (PW_PLAYER);

		/* Heal 0-4 */
		if (num < 5)
		{
			msg_print("You feel a little better.");
		}

		/* Heal 5-14 */
		else if (num < 15)
		{
			msg_print("You feel better.");
		}

		/* Heal 15-34 */
		else if (num < 35)
		{
			msg_print("You feel much better.");
		}

		/* Heal 35+ */
		else
		{
			msg_print("You feel very good.");
		}

		/* Notice */
		return (TRUE);
	}

	/* Ignore */
	return (FALSE);
}



/*
 * Leave a "glyph of warding" which prevents monster movement
 */
void warding_glyph(void)
{
	/* XXX XXX XXX */
	if (!cave_clean_bold(py, px))
	{
		msg_print("The object resists the spell.");
		return;
	}

	/* Create a glyph */
	cave_set_feat(py, px, FEAT_GLYPH);
}

void explosive_rune(void)
{
	/* XXX XXX XXX */
	if (!cave_clean_bold(py, px))
	{
		msg_print("The object resists the spell.");
		return;
	}

	/* Create a glyph */
	cave_set_feat(py, px, FEAT_MINOR_GLYPH);
}



/*
 * Array of stat "descriptions"
 */
static cptr desc_stat_pos[] =
{
	"strong",
	"smart",
	"wise",
	"dextrous",
	"healthy",
	"cute"
};


/*
 * Array of stat "descriptions"
 */
static cptr desc_stat_neg[] =
{
	"weak",
	"stupid",
	"naive",
	"clumsy",
	"sickly",
	"ugly"
};


/*
 * Lose a "point"
 */
bool do_dec_stat(int stat, int mode)
{
	bool sust = FALSE;

	/* Access the "sustain" */
	switch (stat)
	{
		case A_STR: if (p_ptr->sustain_str) sust = TRUE; break;
		case A_INT: if (p_ptr->sustain_int) sust = TRUE; break;
		case A_WIS: if (p_ptr->sustain_wis) sust = TRUE; break;
		case A_DEX: if (p_ptr->sustain_dex) sust = TRUE; break;
		case A_CON: if (p_ptr->sustain_con) sust = TRUE; break;
		case A_CHR: if (p_ptr->sustain_chr) sust = TRUE; break;
	}

	/* Sustain */
	if (sust)
	{
		/* Message */
		msg_format("You feel %s for a moment, but the feeling passes.",
			   desc_stat_neg[stat]);

		/* Notice effect */
		return (TRUE);
	}

	/* Attempt to reduce the stat */
	if (dec_stat(stat, 10, mode))
	{
		/* Message */
		msg_format("You feel very %s.", desc_stat_neg[stat]);

		/* Notice effect */
		return (TRUE);
	}

	/* Nothing obvious */
	return (FALSE);
}


/*
 * Restore lost "points" in a stat
 */
bool do_res_stat(int stat)
{
	/* Attempt to increase */
	if (res_stat(stat))
	{
		/* Message */
		msg_format("You feel less %s.", desc_stat_neg[stat]);

		/* Notice */
		return (TRUE);
	}

	/* Nothing obvious */
	return (FALSE);
}


/*
 * Gain a "point" in a stat
 */
bool do_inc_stat(int stat)
{
	bool res;

	/* Restore strength */
	res = res_stat(stat);

	/* Attempt to increase */
	if (inc_stat(stat))
	{
		/* Message */
		msg_format("Wow!  You feel very %s!", desc_stat_pos[stat]);

		/* Notice */
		return (TRUE);
	}

	/* Restoration worked */
	if (res)
	{
		/* Message */
		msg_format("You feel less %s.", desc_stat_neg[stat]);

		/* Notice */
		return (TRUE);
	}

	/* Nothing obvious */
	return (FALSE);
}



/*
 * Identify everything being carried.
 * Done by a potion of "self knowledge".
 */
void identify_pack(void)
{
	int i;

	/* Simply identify and know every item */
	for (i = 0; i < INVEN_TOTAL; i++)
	{
		object_type *o_ptr = &inventory[i];

		/* Skip non-objects */
		if (!o_ptr->k_idx) continue;

		/* Aware and Known */
		object_aware(o_ptr);
		object_known(o_ptr);

                /* Process the appropriate hooks */
                process_hooks(HOOK_IDENTIFY, i);
	}
}






/*
 * Used by the "enchant" function (chance of failure)
 * (modified for Zangband, we need better stuff there...) -- TY
 */
static int enchant_table[16] =
{
	0, 10,  50, 100, 200,
	300, 400, 500, 650, 800,
	950, 987, 993, 995, 998,
	1000
};


/*
 * Removes curses from items in inventory
 *
 * Note that Items which are "Perma-Cursed" (The One Ring,
 * The Crown of Morgoth) can NEVER be uncursed.
 *
 * Note that if "all" is FALSE, then Items which are
 * "Heavy-Cursed" (Mormegil, Calris, and Weapons of Morgul)
 * will not be uncursed.
 */
static int remove_curse_aux(int all)
{
	int i, cnt = 0;

	/* Attempt to uncurse items being worn */
        for (i = 0; i < INVEN_TOTAL; i++)
	{
                u32b f1, f2, f3, f4, f5, esp;

		object_type *o_ptr = &inventory[i];

		/* Skip non-objects */
		if (!o_ptr->k_idx) continue;

		/* Uncursed already */
		if (!cursed_p(o_ptr)) continue;

		/* Extract the flags */
                object_flags(o_ptr, &f1, &f2, &f3, &f4, &f5, &esp);

		/* Heavily Cursed Items need a special spell */
		if (!all && (f3 & (TR3_HEAVY_CURSE))) continue;

		/* Perma-Cursed Items can NEVER be uncursed */
		if (f3 & (TR3_PERMA_CURSE)) continue;

		/* Uncurse it */
		o_ptr->ident &= ~(IDENT_CURSED);

		/* Hack -- Assume felt */
		o_ptr->ident |= (IDENT_SENSE);

		if (o_ptr->art_flags3 & (TR3_CURSED))
			o_ptr->art_flags3 &= ~(TR3_CURSED);

		if (o_ptr->art_flags3 & (TR3_HEAVY_CURSE))
			o_ptr->art_flags3 &= ~(TR3_HEAVY_CURSE);

		/* Take note */
		o_ptr->sense = SENSE_UNCURSED;

                /* Reverse the curse effect */
 /* jk - scrolls of *remove curse* have a 1 in (55-level chance to */
 /* reverse the curse effects - a ring of damage(-15) {cursed} then */
 /* becomes a ring of damage (+15) */
 /* this does not go for artifacts - a Sword of Mormegil +40,+60 would */
 /* be somewhat unbalancing */
 /* due to the nature of this procedure, it only works on cursed items */
 /* ie you get only one chance! */
                if ((randint(55-p_ptr->lev)==1) && !artifact_p(o_ptr))
                {
                        if (o_ptr->to_a<0) o_ptr->to_a=-o_ptr->to_a;
                        if (o_ptr->to_h<0) o_ptr->to_h=-o_ptr->to_h;
                        if (o_ptr->to_d<0) o_ptr->to_d=-o_ptr->to_d;
                        if (o_ptr->pval<0) o_ptr->pval=-o_ptr->pval;
                }

		/* Recalculate the bonuses */
		p_ptr->update |= (PU_BONUS);

		/* Window stuff */
		p_ptr->window |= (PW_EQUIP);

		/* Count the uncursings */
		cnt++;
	}

	/* Return "something uncursed" */
	return (cnt);
}


/*
 * Remove most curses
 */
bool remove_curse(void)
{
	return (remove_curse_aux(FALSE));
}

/*
 * Remove all curses
 */
bool remove_all_curse(void)
{
	return (remove_curse_aux(TRUE));
}



/*
 * Restores any drained experience
 */
bool restore_level(void)
{
	/* Restore experience */
	if (p_ptr->exp < p_ptr->max_exp)
	{
		/* Message */
		msg_print("You feel your life energies returning.");

		/* Restore the experience */
		p_ptr->exp = p_ptr->max_exp;

		/* Check the experience */
		check_experience();

		/* Did something */
		return (TRUE);
	}

	/* No effect */
	return (FALSE);
}


bool alchemy(void) /* Turns an object into gold, gain some of its value in a shop */
{
	int item, amt = 1;
	int old_number;
	long price;
	bool force = FALSE;
	object_type *o_ptr;
	char o_name[80];
	char out_val[160];

	cptr q, s;

	/* Hack -- force destruction */
	if (command_arg > 0) force = TRUE;

	/* Get an item */
	q = "Turn which item to gold? ";
	s = "You have nothing to turn to gold.";
	if (!get_item(&item, q, s, (USE_INVEN | USE_FLOOR))) return (FALSE);

	/* Get the item (in the pack) */
	if (item >= 0)
	{
		o_ptr = &inventory[item];
	}

	/* Get the item (on the floor) */
	else
	{
		o_ptr = &o_list[0 - item];
	}


	/* See how many items */
	if (o_ptr->number > 1)
	{
		/* Get a quantity */
		amt = get_quantity(NULL, o_ptr->number);

		/* Allow user abort */
	if (amt <= 0) return FALSE;
	}


	/* Describe the object */
	old_number = o_ptr->number;
	o_ptr->number = amt;
	object_desc(o_name, o_ptr, TRUE, 3);
	o_ptr->number = old_number;

	/* Verify unless quantity given */
	if (!force)
	{
		if (!((auto_destroy) && (object_value(o_ptr)<1)))
		{
			/* Make a verification */
			sprintf(out_val, "Really turn %s to gold? ", o_name);
			if (!get_check(out_val)) return FALSE;
		}
	}

	/* Artifacts cannot be destroyed */
        if (artifact_p(o_ptr) || o_ptr->art_name)
	{
                byte feel = SENSE_SPECIAL;

		/* Message */
		msg_format("You fail to turn %s to gold!", o_name);

		/* Hack -- Handle icky artifacts */
                if (cursed_p(o_ptr)) feel = SENSE_TERRIBLE;

		/* Hack -- inscribe the artifact */
		o_ptr->sense = feel;

		/* We have "felt" it (again) */
		o_ptr->ident |= (IDENT_SENSE);

		/* Combine the pack */
		p_ptr->notice |= (PN_COMBINE);

		/* Window stuff */
		p_ptr->window |= (PW_INVEN | PW_EQUIP);

		/* Done */
                return FALSE;
	}

	price = object_value_real(o_ptr);

	if (price <= 0)
		/* Message */
		msg_format("You turn %s to fool's gold.", o_name);
	else
	{
		price /= 3;

		if (amt > 1) price *= amt;

		msg_format("You turn %s to %ld coins worth of gold.", o_name, price);
		p_ptr->au += price;

		/* Redraw gold */
		p_ptr->redraw |= (PR_GOLD);

		/* Window stuff */
		p_ptr->window |= (PW_PLAYER);

	}

	/* Eliminate the item (from the pack) */
	if (item >= 0)
	{
		inven_item_increase(item, -amt);
		inven_item_describe(item);
		inven_item_optimize(item);
	}

	/* Eliminate the item (from the floor) */
	else
	{
		floor_item_increase(0 - item, -amt);
		floor_item_describe(0 - item);
		floor_item_optimize(0 - item);
	}

	return TRUE;
}




/*
 * self-knowledge... idea from nethack.  Useful for determining powers and
 * resistences of items.  It saves the screen, clears it, then starts listing
 * attributes, a screenful at a time.  (There are a LOT of attributes to
 * list.  It will probably take 2 or 3 screens for a powerful character whose
 * using several artifacts...) -CFT
 *
 * It is now a lot more efficient. -BEN-
 *
 * See also "identify_fully()".
 *
 * XXX XXX XXX Use the "show_file()" method, perhaps.
 */
void self_knowledge(FILE *fff)
{
	int i = 0, j, k;

        u32b f1 = 0L, f2 = 0L, f3 = 0L, f4, f5, esp;

	int iter; /* Iterator for a loop */

	object_type *o_ptr;

	char Dummy[80];

	cptr info[200];

	strcpy (Dummy, "");

	/* Acquire item flags from equipment */
	for (k = INVEN_WIELD; k < INVEN_TOTAL; k++)
	{
                u32b t1, t2, t3, t4, t5, esp;

		o_ptr = &inventory[k];

		/* Skip non-objects */
		if (!o_ptr->k_idx) continue;

		/* Extract the flags */
                object_flags(o_ptr, &t1, &t2, &t3, &t4, &t5, &esp);

		/* Extract flags */
		f1 |= t1;
		f2 |= t2;
		f3 |= t3;
	}


	/* Racial powers... */
	if (p_ptr->body_monster != 0)
	{
		monster_race *r_ptr = &r_info[p_ptr->body_monster];

		if (r_ptr->flags1 & RF1_CHAR_CLEAR ||
		    r_ptr->flags1 & RF1_ATTR_CLEAR)
			info[i++] = "You are clear.";
		if ((r_ptr->flags1 & RF1_CHAR_MULTI) ||
		    (r_ptr->flags2 & RF2_SHAPECHANGER))
			info[i++] = "Your form constantly change.";
		if (r_ptr->flags1 & RF1_ATTR_MULTI)
			info[i++] = "Your color constantly change.";
		if (r_ptr->flags1 & RF1_NEVER_BLOW)
			info[i++] = "You do not have any physical weapon";
		if (r_ptr->flags1 & RF1_NEVER_MOVE)
			info[i++] = "You cannot move.";
		if ((r_ptr->flags1 & RF1_RAND_25) &&
		    (r_ptr->flags1 & RF1_RAND_50))
			info[i++] = "You move extremely erratically.";
		else if (r_ptr->flags1 & RF1_RAND_50)
		        info[i++] = "You move somewhat erratically.";
		else if (r_ptr->flags1 & RF1_RAND_25)
			info[i++] = "You move a bit erratically.";
		if (r_ptr->flags2 & RF2_STUPID)
			info[i++] = "You are very stupid (INT -4).";
		if (r_ptr->flags2 & RF2_SMART)
			info[i++] = "You are very smart (INT +4).";
		/* Not implemented */
		if (r_ptr->flags2 & RF2_CAN_SPEAK)
			info[i++] = "You can speak.";
		else
			info[i++] = "You cannot speak.";
		/* Not implemented */
		if (r_ptr->flags2 & RF2_COLD_BLOOD)
			info[i++] = "You are cold blooded.";
		/* Not implemented */
		if (r_ptr->flags2 & RF2_EMPTY_MIND)
			info[i++] = "You have empty mind.";
		if (r_ptr->flags2 & RF2_WEIRD_MIND)
			info[i++] = "You have weird mind.";
                if (r_ptr->flags4 & RF4_MULTIPLY)
			info[i++] = "You can multiply.";	
		if (r_ptr->flags2 & RF2_POWERFUL)
			info[i++] = "You have strong breath.";
		/* Not implemented */
		if (r_ptr->flags2 & RF2_ELDRITCH_HORROR)
			info[i++] = "You are an eldritch horror.";
		if (r_ptr->flags2 & RF2_OPEN_DOOR)
			info[i++] = "You can open doors.";
		else
			info[i++] = "You cannot open doors.";
		if (r_ptr->flags2 & RF2_BASH_DOOR)
			info[i++] = "You can bash doors.";
		else
			info[i++] = "You cannot bash doors.";
		if (r_ptr->flags2 & RF2_PASS_WALL)
			info[i++] = "You can pass walls.";
		if (r_ptr->flags2 & RF2_KILL_WALL)
			info[i++] = "You destroy walls.";
		/* Not implemented */
		if (r_ptr->flags2 & RF2_MOVE_BODY)
			info[i++] = "You can move monsters.";
		/* Not implemented */
#if 0
		/* They are disabled, because the r_info.txt array has to
		 * few RF2_TAKE_ITEM flags... */
		if (r_ptr->flags2 & RF2_TAKE_ITEM)
			info[i++] = "You can pick up items.";
		else
			info[i++] = "You cannot pick up items.";
#endif
		/* Not implemented */
		if (r_ptr->flags3 & RF3_ORC)
			info[i++] = "You have orc blood in your veins.";
		/* Not implemented */
		else if (r_ptr->flags3 & RF3_TROLL)
			info[i++] = "You have troll blood in your veins.";
		/* Not implemented */
		else if (r_ptr->flags3 & RF3_GIANT)
			info[i++] = "You have giant blood in your veins.";
		/* Not implemented */
		else if (r_ptr->flags3 & RF3_DRAGON)
			info[i++] = "You have dragon blood in your veins.";
		/* Not implemented */
		else if (r_ptr->flags3 & RF3_DEMON)
			info[i++] = "You have demon blood in your veins.";
		/* Not implemented */
		else if (r_ptr->flags3 & RF3_UNDEAD)
			info[i++] = "You are an undead.";
		/* Not implemented */
		else if (r_ptr->flags3 & RF3_ANIMAL)
			info[i++] = "You are an animal.";
		/* Not implemented */
		else if (r_ptr->flags3 & RF3_DRAGONRIDDER)
			info[i++] = "You have dragonrider blood in your veins.";
		if (r_ptr->flags3 & RF3_EVIL)
			info[i++] = "You are inherently evil.";
		else if (r_ptr->flags3 & RF3_GOOD)
			info[i++] = "You are inherently good.";
		if (r_ptr->flags3 & RF3_AURA_COLD)
			info[i++] = "You are surrounded by a chilly aura.";
		/* Not implemented */
		if (r_ptr->flags3 & RF3_NONLIVING)
			info[i++] = "You are not living.";
		/* Not implemented */
		if (r_ptr->flags3 & RF3_HURT_LITE)
			info[i++] = "Your eyes are sensible to bright light.";
		/* Not implemented */
		if (r_ptr->flags3 & RF3_HURT_ROCK)
			info[i++] = "You can be hurt by rock remover.";
		if (r_ptr->flags3 & RF3_SUSCEP_FIRE)
			info[i++] = "You are sensitive to fire.";
		if (r_ptr->flags3 & RF3_SUSCEP_COLD)
			info[i++] = "You are sensitive to cold.";
		if (r_ptr->flags3 & RF3_RES_TELE)
			info[i++] = "You are resistant to teleportation.";
		if (r_ptr->flags3 & RF3_RES_NETH)
			info[i++] = "You are resistant to nether.";
		if (r_ptr->flags3 & RF3_RES_WATE)
			info[i++] = "You are resistant to water.";
		if (r_ptr->flags3 & RF3_RES_PLAS)
			info[i++] = "You are resistant to plasma.";
		if (r_ptr->flags3 & RF3_RES_WATE)
			info[i++] = "You are resistant to nexus.";
		if (r_ptr->flags3 & RF3_RES_DISE)
			info[i++] = "You are resistant to disease.";
		/* Not implemented */
		if (r_ptr->flags3 & RF3_NO_SLEEP)
			info[i++] = "You cannot be slept.";
		/* Not implemented */
		if (r_ptr->flags3 & RF3_UNIQUE_4)
			info[i++] = "You are a Nazgul.";
		if (r_ptr->flags3 & RF3_NO_FEAR)
			info[i++] = "You are immune to fear.";
		if (r_ptr->flags3 & RF3_NO_STUN)
			info[i++] = "You are immune to stun.";
		if (r_ptr->flags3 & RF3_NO_CONF)
			info[i++] = "You are immune to confusion.";
		if (r_ptr->flags3 & RF3_NO_SLEEP)
			info[i++] = "You are immune to sleep.";

		if (r_ptr->flags4 & RF4_SHRIEK)
			info[i++] = "You can aggrevate monsters.";
		if (r_ptr->flags4 & RF4_ROCKET)
			info[i++] = "You can fire a rocket.";
		if (r_ptr->flags4 & RF4_ARROW_1)
			info[i++] = "You can fire a light arrow.";
		if (r_ptr->flags4 & RF4_ARROW_2)
			info[i++] = "You can fire a heavy arrow.";
		if (r_ptr->flags4 & RF4_ARROW_3)
			info[i++] = "You can fire a light missile.";
		if (r_ptr->flags4 & RF4_ARROW_4)
			info[i++] = "You can fire a heavy missile.";
		if (r_ptr->flags4 & RF4_BR_ACID)
			info[i++] = "You can breathe acid.";
		if (r_ptr->flags4 & RF4_BR_ELEC)
			info[i++] = "You can breathe electricity.";
		if (r_ptr->flags4 & RF4_BR_FIRE)
			info[i++] = "You can breathe fire.";
		if (r_ptr->flags4 & RF4_BR_COLD)
			info[i++] = "You can breathe cold.";
		if (r_ptr->flags4 & RF4_BR_POIS)
			info[i++] = "You can breathe poison.";
		if (r_ptr->flags4 & RF4_BR_NETH)
			info[i++] = "You can breathe nether.";
		if (r_ptr->flags4 & RF4_BR_LITE)
			info[i++] = "You can breathe light.";
		if (r_ptr->flags4 & RF4_BR_DARK)
			info[i++] = "You can breathe darkness.";
		if (r_ptr->flags4 & RF4_BR_CONF)
			info[i++] = "You can breathe confusion.";
		if (r_ptr->flags4 & RF4_BR_SOUN)
			info[i++] = "You can breathe sound.";
		if (r_ptr->flags4 & RF4_BR_CHAO)
			info[i++] = "You can breathe chaos.";
		if (r_ptr->flags4 & RF4_BR_DISE)
			info[i++] = "You can breathe disenchantment.";
		if (r_ptr->flags4 & RF4_BR_NEXU)
			info[i++] = "You can breathe nexus.";
		if (r_ptr->flags4 & RF4_BR_TIME)
			info[i++] = "You can breathe time.";
		if (r_ptr->flags4 & RF4_BR_INER)
			info[i++] = "You can breathe inertia.";
		if (r_ptr->flags4 & RF4_BR_GRAV)
			info[i++] = "You can breathe gravity.";
		if (r_ptr->flags4 & RF4_BR_SHAR)
			info[i++] = "You can breathe shards.";
		if (r_ptr->flags4 & RF4_BR_PLAS)
			info[i++] = "You can breathe plasma.";
		if (r_ptr->flags4 & RF4_BR_WALL)
			info[i++] = "You can breathe force.";
		if (r_ptr->flags4 & RF4_BR_MANA)
			info[i++] = "You can breathe mana.";
		if (r_ptr->flags4 & RF4_BR_NUKE)
			info[i++] = "You can breathe nuke.";
		if (r_ptr->flags4 & RF4_BR_DISI)
			info[i++] = "You can breathe disitegration.";
		if (r_ptr->flags5 & RF5_BA_ACID)
			info[i++] = "You can cast a ball of acid.";
		if (r_ptr->flags5 & RF5_BA_ELEC)
			info[i++] = "You can cast a ball of electricity.";
		if (r_ptr->flags5 & RF5_BA_FIRE)
			info[i++] = "You can cast a ball of fire.";
		if (r_ptr->flags5 & RF5_BA_COLD)
			info[i++] = "You can cast a ball of cold.";
		if (r_ptr->flags5 & RF5_BA_POIS)
			info[i++] = "You can cast a ball of poison.";
		if (r_ptr->flags5 & RF5_BA_NETH)
			info[i++] = "You can cast a ball of nether.";
		if (r_ptr->flags5 & RF5_BA_WATE)
			info[i++] = "You can cast a ball of water.";
		/* Not implemented */
		if (r_ptr->flags5 & RF5_DRAIN_MANA)
			info[i++] = "You can drain mana.";
		if (r_ptr->flags5 & RF5_MIND_BLAST)
			info[i++] = "You can cause mind blasting.";
		if (r_ptr->flags5 & RF5_BRAIN_SMASH)
			info[i++] = "You can cause brain smashing.";
		if (r_ptr->flags5 & RF5_CAUSE_1)
			info[i++] = "You can cause light wounds.";
		if (r_ptr->flags5 & RF5_CAUSE_2)
			info[i++] = "You can cause serious wounds.";
		if (r_ptr->flags5 & RF5_CAUSE_3)
			info[i++] = "You can cause critical wounds.";
		if (r_ptr->flags5 & RF5_CAUSE_4)
			info[i++] = "You can cause mortal wounds.";
		if (r_ptr->flags5 & RF5_BO_ACID)
			info[i++] = "You can cast a bolt of acid.";
		if (r_ptr->flags5 & RF5_BO_ELEC)
			info[i++] = "You can cast a bolt of electricity.";
		if (r_ptr->flags5 & RF5_BO_FIRE)
			info[i++] = "You can cast a bolt of fire.";
		if (r_ptr->flags5 & RF5_BO_COLD)
			info[i++] = "You can cast a bolt of cold.";
		if (r_ptr->flags5 & RF5_BO_POIS)
			info[i++] = "You can cast a bolt of poison.";
		if (r_ptr->flags5 & RF5_BO_NETH)
			info[i++] = "You can cast a bolt of nether.";
		if (r_ptr->flags5 & RF5_BO_WATE)
			info[i++] = "You can cast a bolt of water.";
		if (r_ptr->flags5 & RF5_BO_MANA)
			info[i++] = "You can cast a bolt of mana.";
		if (r_ptr->flags5 & RF5_BO_PLAS)
			info[i++] = "You can cast a bolt of plasma.";
		if (r_ptr->flags5 & RF5_BO_ICEE)
			info[i++] = "You can cast a bolt of ice.";
		if (r_ptr->flags5 & RF5_MISSILE)
			info[i++] = "You can cast magic missile.";
		if (r_ptr->flags5 & RF5_SCARE)
			info[i++] = "You can terrify.";
		if (r_ptr->flags5 & RF5_BLIND)
			info[i++] = "You can blind.";
		if (r_ptr->flags5 & RF5_CONF)
			info[i++] = "You can use confusion.";
		if (r_ptr->flags5 & RF5_SLOW)
			info[i++] = "You can cast slow.";
		if (r_ptr->flags5 & RF5_HOLD)
			info[i++] = "You can touch to paralyze.";
		if (r_ptr->flags6 & RF6_HASTE)
			info[i++] = "You can haste yourself.";
		if (r_ptr->flags6 & RF6_HAND_DOOM)
			info[i++] = "You can invoke Hand of Doom.";
		if (r_ptr->flags6 & RF6_HEAL)
			info[i++] = "You can heal yourself.";
		if (r_ptr->flags6 & RF6_BLINK)
			info[i++] = "You can blink.";
		if (r_ptr->flags6 & RF6_TPORT)
			info[i++] = "You can teleport.";
		if (r_ptr->flags6 & RF6_TELE_TO)
			info[i++] = "You can go between.";
		if (r_ptr->flags6 & RF6_TELE_AWAY)
			info[i++] = "You can teleport away.";
		if (r_ptr->flags6 & RF6_TELE_LEVEL)
			info[i++] = "You can teleport level.";
		if (r_ptr->flags6 & RF6_DARKNESS)
			info[i++] = "You can create darkness.";
		if (r_ptr->flags6 & RF6_TRAPS)
			info[i++] = "You can create traps.";
		/* Not implemented */
		if (r_ptr->flags6 & RF6_FORGET)
			info[i++] = "You can fade memories.";
		if (r_ptr->flags6 & RF6_RAISE_DEAD)
			info[i++] = "You can Raise the Dead.";
		if (r_ptr->flags6 & RF6_S_BUG)
			info[i++] = "You can magically summon a Software Bugs.";
		if (r_ptr->flags6 & RF6_S_RNG)
			info[i++] = "You can magically summon the RNG.";
		if (r_ptr->flags6 & RF6_S_DRAGONRIDDER)
			info[i++] = "You can magically summon some Dragonriders.";
		if (r_ptr->flags6 & RF6_S_KIN)
			info[i++] = "You can magically summon some Kins.";
                if (r_ptr->flags6 & RF6_S_HI_DEMON)
                        info[i++] = "You can magically summon greater demons.";
		if (r_ptr->flags6 & RF6_S_MONSTER)
			info[i++] = "You can magically summon a monster.";
		if (r_ptr->flags6 & RF6_S_MONSTERS)
			info[i++] = "You can magically summon monsters.";
		if (r_ptr->flags6 & RF6_S_ANT)
			info[i++] = "You can magically summon ants.";
		if (r_ptr->flags6 & RF6_S_SPIDER)
			info[i++] = "You can magically summon spiders.";
		if (r_ptr->flags6 & RF6_S_HOUND)
			info[i++] = "You can magically summon hounds.";
		if (r_ptr->flags6 & RF6_S_HYDRA)
			info[i++] = "You can magically summon hydras.";
		if (r_ptr->flags6 & RF6_S_ANGEL)
			info[i++] = "You can magically summon an angel.";
		if (r_ptr->flags6 & RF6_S_DEMON)
			info[i++] = "You can magically summon a demon.";
		if (r_ptr->flags6 & RF6_S_UNDEAD)
			info[i++] = "You can magically summon an undead.";
		if (r_ptr->flags6 & RF6_S_DRAGON)
			info[i++] = "You can magically summon a dragon.";
		if (r_ptr->flags6 & RF6_S_HI_UNDEAD)
			info[i++] = "You can magically summon greater undead.";
		if (r_ptr->flags6 & RF6_S_HI_DRAGON)
			info[i++] = "You can magically summon greater dragons.";
		if (r_ptr->flags6 & RF6_S_WRAITH)
			info[i++] = "You can magically summon a wraith.";
		/* Not implemented */
		if (r_ptr->flags6 & RF6_S_UNIQUE)
			info[i++] = "You can magically summon an unique monster.";
		/* Not implemented */
		if (r_ptr->flags7 & RF7_AQUATIC)
			info[i++] = "You are aquatic.";
		/* Not implemented */
		if (r_ptr->flags7 & RF7_CAN_SWIM)
			info[i++] = "You can swim.";
		/* Not implemented */
		if (r_ptr->flags7 & RF7_CAN_FLY)
			info[i++] = "You can fly.";
		if ((r_ptr->flags7 & RF7_MORTAL) == 0)
			info[i++] = "You are immortal.";
		/* Not implemented */
		if (r_ptr->flags7 & RF7_NAZGUL)
			info[i++] = "You are a Nazgul.";

                if (r_ptr->flags7 & RF7_SPIDER)
                        info[i++] = "You are a spider.";

                if (r_ptr->flags8 & RF8_WILD_TOWN)
			info[i++] = "You appear in towns.";
		if (r_ptr->flags8 & RF8_WILD_SHORE)
			info[i++] = "You appear in shores.";
		if (r_ptr->flags8 & RF8_WILD_OCEAN)
			info[i++] = "You appear in the ocean.";
		if (r_ptr->flags8 & RF8_WILD_WASTE)
			info[i++] = "You appear in the waste.";
		if (r_ptr->flags8 & RF8_WILD_WOOD)
			info[i++] = "You appear in woods.";
		if (r_ptr->flags8 & RF8_WILD_VOLCANO)
			info[i++] = "You appear in volcanos.";
		if (r_ptr->flags8 & RF8_WILD_MOUNTAIN)
			info[i++] = "You appear in the mountains.";
		if (r_ptr->flags8 & RF8_WILD_GRASS)
			info[i++] = "You appear in grasses.";
		
		if (r_ptr->flags9 & RF9_SUSCEP_ACID)
			info[i++] = "You are sensitive to acid.";
		if (r_ptr->flags9 & RF9_SUSCEP_ELEC)
			info[i++] = "You are sensitive to electricity.";
		if (r_ptr->flags9 & RF9_SUSCEP_POIS)
			info[i++] = "You are sensitive to poison.";
		if (r_ptr->flags9 & RF9_KILL_TREES)
			info[i++] = "You can eat trees.";
		if (r_ptr->flags9 & RF9_WYRM_PROTECT)
			info[i++] = "You are protected by great wyrms of power.";
	}

        /* List powers */
        for (iter = 0; iter < POWER_MAX; iter++)
        {
                if (p_ptr->powers[iter / 32] & BIT(iter % 32))
                {
                        info[i++] = powers_type[iter].desc_text;
                }
        }

	if (p_ptr->muta2)
	{
		if (p_ptr->muta2 & MUT2_BERS_RAGE)
		{
			info[i++] = "You are subject to berserker fits.";
		}
		if (p_ptr->muta2 & MUT2_COWARDICE)
		{
			info[i++] = "You are subject to cowardice.";
		}
		if (p_ptr->muta2 & MUT2_RTELEPORT)
		{
			info[i++] = "You are teleporting randomly.";
		}
		if (p_ptr->muta2 & MUT2_ALCOHOL)
		{
			info[i++] = "Your body produces alcohol.";
		}
		if (p_ptr->muta2 & MUT2_HALLU)
		{
			info[i++] = "You have a hallucinatory insanity.";
		}
		if (p_ptr->muta2 & MUT2_FLATULENT)
		{
			info[i++] = "You are subject to uncontrollable flatulence.";
		}
		if (p_ptr->muta2 & MUT2_PROD_MANA)
		{
			info[i++] = "You are producing magical energy uncontrollably.";
		}
		if (p_ptr->muta2 & MUT2_ATT_DEMON)
		{
			info[i++] = "You attract demons.";
		}
		if (p_ptr->muta2 & MUT2_SCOR_TAIL)
		{
			info[i++] = "You have a scorpion tail (poison, 3d7).";
		}
		if (p_ptr->muta2 & MUT2_HORNS)
		{
			info[i++] = "You have horns (dam. 2d6).";
		}
		if (p_ptr->muta2 & MUT2_BEAK)
		{
			info[i++] = "You have a beak (dam. 2d4).";
		}
		if (p_ptr->muta2 & MUT2_SPEED_FLUX)
		{
			info[i++] = "You move faster or slower randomly.";
		}
		if (p_ptr->muta2 & MUT2_BANISH_ALL)
		{
			info[i++] = "You sometimes cause nearby creatures to vanish.";
		}
		if (p_ptr->muta2 & MUT2_EAT_LIGHT)
		{
			info[i++] = "You sometimes feed off of the light around you.";
		}
		if (p_ptr->muta2 & MUT2_TRUNK)
		{
			info[i++] = "You have an elephantine trunk (dam 1d4).";
		}
		if (p_ptr->muta2 & MUT2_ATT_ANIMAL)
		{
			info[i++] = "You attract animals.";
		}
		if (p_ptr->muta2 & MUT2_TENTACLES)
		{
			info[i++] = "You have evil looking tentacles (dam 2d5).";
		}
		if (p_ptr->muta2 & MUT2_RAW_CHAOS)
		{
			info[i++] = "You occasionally are surrounded with raw chaos.";
		}
		if (p_ptr->muta2 & MUT2_NORMALITY)
		{
                        info[i++] = "You may be corrupted, but you're recovering.";
		}
		if (p_ptr->muta2 & MUT2_WRAITH)
		{
			info[i++] = "You fade in and out of physical reality.";
		}
		if (p_ptr->muta2 & MUT2_POLY_WOUND)
		{
			info[i++] = "Your health is subject to chaotic forces.";
		}
		if (p_ptr->muta2 & MUT2_WASTING)
		{
			info[i++] = "You have a horrible wasting disease.";
		}
		if (p_ptr->muta2 & MUT2_ATT_DRAGON)
		{
			info[i++] = "You attract dragons.";
		}
		if (p_ptr->muta2 & MUT2_WEIRD_MIND)
		{
			info[i++] = "Your mind randomly expands and contracts.";
		}
		if (p_ptr->muta2 & MUT2_NAUSEA)
		{
			info[i++] = "You have a seriously upset stomach.";
		}
		if (p_ptr->muta2 & MUT2_CHAOS_GIFT)
		{
			info[i++] = "Chaos deities give you gifts.";
		}
		if (p_ptr->muta2 & MUT2_WALK_SHAD)
		{
			info[i++] = "You occasionally stumble into other shadows.";
		}
		if (p_ptr->muta2 & MUT2_WARNING)
		{
			info[i++] = "You receive warnings about your foes.";
		}
		if (p_ptr->muta2 & MUT2_INVULN)
		{
			info[i++] = "You occasionally feel invincible.";
		}
		if (p_ptr->muta2 & MUT2_SP_TO_HP)
		{
			info[i++] = "Your blood sometimes rushes to your muscles.";
		}
		if (p_ptr->muta2 & MUT2_HP_TO_SP)
		{
			info[i++] = "Your blood sometimes rushes to your head.";
		}
		if (p_ptr->muta2 & MUT2_DISARM)
		{
			info[i++] = "You occasionally stumble and drop things.";
		}
	}

	if (p_ptr->muta3)
	{
		if (p_ptr->muta3 & MUT3_HYPER_STR)
		{
			info[i++] = "You are superhumanly strong (+4 STR).";
		}
		if (p_ptr->muta3 & MUT3_PUNY)
		{
			info[i++] = "You are puny (-4 STR).";
		}
		if (p_ptr->muta3 & MUT3_HYPER_INT)
		{
			info[i++] = "Your brain is a living computer (+4 INT/WIS).";
		}
		if (p_ptr->muta3 & MUT3_MORONIC)
		{
			info[i++] = "You are moronic (-4 INT/WIS).";
		}
		if (p_ptr->muta3 & MUT3_RESILIENT)
		{
			info[i++] = "You are very resilient (+4 CON).";
		}
		if (p_ptr->muta3 & MUT3_XTRA_FAT)
		{
			info[i++] = "You are extremely fat (+2 CON, -2 speed).";
		}
		if (p_ptr->muta3 & MUT3_ALBINO)
		{
			info[i++] = "You are albino (-4 CON).";
		}
		if (p_ptr->muta3 & MUT3_FLESH_ROT)
		{
			info[i++] = "Your flesh is rotting (-2 CON, -1 CHR).";
		}
		if (p_ptr->muta3 & MUT3_SILLY_VOI)
		{
			info[i++] = "Your voice is a silly squeak (-4 CHR).";
		}
		if (p_ptr->muta3 & MUT3_BLANK_FAC)
		{
			info[i++] = "Your face is featureless (-1 CHR).";
		}
		if (p_ptr->muta3 & MUT3_ILL_NORM)
		{
			info[i++] = "Your appearance is masked with illusion.";
		}
		if (p_ptr->muta3 & MUT3_XTRA_EYES)
		{
			info[i++] = "You have an extra pair of eyes (+15 search).";
		}
		if (p_ptr->muta3 & MUT3_MAGIC_RES)
		{
			info[i++] = "You are resistant to magic.";
		}
		if (p_ptr->muta3 & MUT3_XTRA_NOIS)
		{
			info[i++] = "You make a lot of strange noise (-3 stealth).";
		}
		if (p_ptr->muta3 & MUT3_INFRAVIS)
		{
			info[i++] = "You have remarkable infravision (+3).";
		}
		if (p_ptr->muta3 & MUT3_XTRA_LEGS)
		{
			info[i++] = "You have an extra pair of legs (+3 speed).";
		}
		if (p_ptr->muta3 & MUT3_SHORT_LEG)
		{
			info[i++] = "Your legs are short stubs (-3 speed).";
		}
		if (p_ptr->muta3 & MUT3_ELEC_TOUC)
		{
			info[i++] = "Electricity is running through your veins.";
		}
		if (p_ptr->muta3 & MUT3_FIRE_BODY)
		{
#if 0
			/* Unnecessary, actually... */
			info[i++] = "Your body is enveloped in flames.";
#endif
		}
		if (p_ptr->muta3 & MUT3_WART_SKIN)
		{
			info[i++] = "Your skin is covered with warts (-2 CHR, +5 AC).";
		}
		if (p_ptr->muta3 & MUT3_SCALES)
		{
			info[i++] = "Your skin has turned into scales (-1 CHR, +10 AC).";
		}
		if (p_ptr->muta3 & MUT3_IRON_SKIN)
		{
			info[i++] = "Your skin is made of steel (-1 DEX, +25 AC).";
		}
		if (p_ptr->muta3 & MUT3_WINGS)
		{
			info[i++] = "You have wings.";
		}
		if (p_ptr->muta3 & MUT3_LIMBER)
		{
			info[i++] = "Your body is very limber (+3 DEX).";
		}
		if (p_ptr->muta3 & MUT3_ARTHRITIS)
		{
			info[i++] = "Your joints ache constantly (-3 DEX).";
		}
		if (p_ptr->muta3 & MUT3_RES_TIME)
		{
			info[i++] = "You are protected from the ravages of time.";
		}
		if (p_ptr->muta3 & MUT3_VULN_ELEM)
		{
			info[i++] = "You are susceptible to damage from the elements.";
		}
		if (p_ptr->muta3 & MUT3_MOTION)
		{
			info[i++] = "Your movements are precise and forceful (+1 STL).";
		}
		if (p_ptr->muta3 & MUT3_SUS_STATS)
		{
			/* Unnecessary */
		}
	}

        if (p_ptr->allow_one_death)
        {
                info[i++] = "The blood of life flows through your veins.";
        }
	if (p_ptr->blind)
	{
		info[i++] = "You cannot see.";
	}
	if (p_ptr->confused)
	{
		info[i++] = "You are confused.";
	}
	if (p_ptr->afraid)
	{
		info[i++] = "You are terrified.";
	}
	if (p_ptr->cut)
	{
		info[i++] = "You are bleeding.";
	}
	if (p_ptr->stun)
	{
		info[i++] = "You are stunned.";
	}
	if (p_ptr->poisoned)
	{
		info[i++] = "You are poisoned.";
	}
	if (p_ptr->image)
	{
		info[i++] = "You are hallucinating.";
	}
	if (p_ptr->aggravate)
	{
		info[i++] = "You aggravate monsters.";
	}
	if (p_ptr->teleport)
	{
		info[i++] = "Your position is very uncertain.";
	}
	if (p_ptr->blessed)
	{
		info[i++] = "You feel rightous.";
	}
	if (p_ptr->hero)
	{
		info[i++] = "You feel heroic.";
	}
	if (p_ptr->shero)
	{
		info[i++] = "You are in a battle rage.";
	}
	if (p_ptr->protevil)
	{
		info[i++] = "You are protected from evil.";
	}
	if (p_ptr->shield)
	{
		info[i++] = "You are protected by a mystic shield.";
	}
	if (p_ptr->invuln)
	{
		info[i++] = "You are temporarily invulnerable.";
	}
	if (p_ptr->confusing)
	{
		info[i++] = "Your hands are glowing dull red.";
	}
	if (p_ptr->searching)
	{
		info[i++] = "You are looking around very carefully.";
	}
	if (p_ptr->new_spells)
	{
		info[i++] = "You can learn some spells/prayers.";
	}
	if (p_ptr->word_recall)
	{
		info[i++] = "You will soon be recalled.";
	}
	if (p_ptr->see_infra)
	{
		info[i++] = "Your eyes are sensitive to infrared light.";
	}
	if (p_ptr->see_inv)
	{
		info[i++] = "You can see invisible creatures.";
	}
	if (p_ptr->ffall)
	{
		info[i++] = "You can fly.";
	}
        if (p_ptr->climb)
	{
                info[i++] = "You can climb hight mountains.";
	}
	if (p_ptr->free_act)
	{
		info[i++] = "You have free action.";
	}
	if (p_ptr->regenerate)
	{
		info[i++] = "You regenerate quickly.";
	}
	if (p_ptr->slow_digest)
	{
		info[i++] = "Your appetite is small.";
	}
	if (p_ptr->telepathy)
	{
                if (p_ptr->telepathy & ESP_ALL) info[i++] = "You have ESP.";
                else
                {
                        if (p_ptr->telepathy & ESP_ORC) info[i++] = "You can sense the presence of orcs.";
                        if (p_ptr->telepathy & ESP_TROLL) info[i++] = "You can sense the presence of trolls.";
                        if (p_ptr->telepathy & ESP_DRAGON) info[i++] = "You can sense the presence of dragons.";
                        if (p_ptr->telepathy & ESP_SPIDER) info[i++] = "You can sense the presence of spiders.";
                        if (p_ptr->telepathy & ESP_GIANT) info[i++] = "You can sense the presence of giants.";
                        if (p_ptr->telepathy & ESP_DEMON) info[i++] = "You can sense the presence of demons.";
                        if (p_ptr->telepathy & ESP_UNDEAD) info[i++] = "You can sense presence of undead.";
                        if (p_ptr->telepathy & ESP_EVIL) info[i++] = "You can sense the presence of evil beings.";
                        if (p_ptr->telepathy & ESP_ANIMAL) info[i++] = "You can sense the presence of animals.";
                        if (p_ptr->telepathy & ESP_DRAGONRIDDER) info[i++] = "You can sense the presence of dragonriders.";
                        if (p_ptr->telepathy & ESP_GOOD) info[i++] = "You can sense the presence of good beings.";
                        if (p_ptr->telepathy & ESP_NONLIVING) info[i++] = "You can sense the presence of non-living things.";
                        if (p_ptr->telepathy & ESP_UNIQUE) info[i++] = "You can sense the presence of unique beings.";
                }
	}
        if (p_ptr->auto_id)
	{
                info[i++] = "You know everything.";
	}
	if (p_ptr->hold_life)
	{
		info[i++] = "You have a firm hold on your life force.";
	}
	if (p_ptr->reflect)
	{
		info[i++] = "You reflect arrows and bolts.";
	}
	if (p_ptr->sh_fire)
	{
		info[i++] = "You are surrounded with a fiery aura.";
	}
	if (p_ptr->sh_elec)
	{
		info[i++] = "You are surrounded with electricity.";
	}
        if (p_ptr->antimagic)
	{
                info[i++] = "You are surrounded by an anti-magic field.";
	}
	if (p_ptr->anti_magic)
	{
		info[i++] = "You are surrounded by an anti-magic shell.";
	}
        if (p_ptr->wraith_form)
	{
                info[i++] = "You are incorporeal.";
	}
	if (p_ptr->anti_tele)
	{
		info[i++] = "You cannot teleport.";
	}
	if (p_ptr->lite)
	{
		info[i++] = "You are carrying a permanent light.";
	}

	if (p_ptr->immune_acid)
	{
		info[i++] = "You are completely immune to acid.";
	}
	else if ((p_ptr->resist_acid) && (p_ptr->oppose_acid))
	{
		info[i++] = "You resist acid exceptionally well.";
	}
	else if ((p_ptr->resist_acid) || (p_ptr->oppose_acid))
	{
		info[i++] = "You are resistant to acid.";
	}

	if (p_ptr->immune_elec)
	{
		info[i++] = "You are completely immune to lightning.";
	}
	else if ((p_ptr->resist_elec) && (p_ptr->oppose_elec))
	{
		info[i++] = "You resist lightning exceptionally well.";
	}
	else if ((p_ptr->resist_elec) || (p_ptr->oppose_elec))
	{
		info[i++] = "You are resistant to lightning.";
	}

	if (p_ptr->immune_fire)
	{
		info[i++] = "You are completely immune to fire.";
	}
	else if ((p_ptr->resist_fire) && (p_ptr->oppose_fire))
	{
		info[i++] = "You resist fire exceptionally well.";
	}
	else if ((p_ptr->resist_fire) || (p_ptr->oppose_fire))
	{
		info[i++] = "You are resistant to fire.";
	}
        else if (p_ptr->sensible_fire)
	{
                info[i++] = "You are very sensible to fire.";
	}

	if (p_ptr->immune_cold)
	{
		info[i++] = "You are completely immune to cold.";
	}
	else if ((p_ptr->resist_cold) && (p_ptr->oppose_cold))
	{
		info[i++] = "You resist cold exceptionally well.";
	}
	else if ((p_ptr->resist_cold) || (p_ptr->oppose_cold))
	{
		info[i++] = "You are resistant to cold.";
	}

	if ((p_ptr->resist_pois) && (p_ptr->oppose_pois))
	{
		info[i++] = "You resist poison exceptionally well.";
	}
	else if ((p_ptr->resist_pois) || (p_ptr->oppose_pois))
	{
		info[i++] = "You are resistant to poison.";
	}

	if (p_ptr->resist_lite)
	{
		info[i++] = "You are resistant to bright light.";
	}
	if (p_ptr->resist_dark)
	{
		info[i++] = "You are resistant to darkness.";
	}
	if (p_ptr->resist_conf)
	{
		info[i++] = "You are resistant to confusion.";
	}
	if (p_ptr->resist_sound)
	{
		info[i++] = "You are resistant to sonic attacks.";
	}
	if (p_ptr->resist_disen)
	{
		info[i++] = "You are resistant to disenchantment.";
	}
	if (p_ptr->resist_chaos)
	{
		info[i++] = "You are resistant to chaos.";
	}
	if (p_ptr->resist_shard)
	{
		info[i++] = "You are resistant to blasts of shards.";
	}
	if (p_ptr->resist_nexus)
	{
		info[i++] = "You are resistant to nexus attacks.";
	}
        if (p_ptr->immune_neth)
	{
                info[i++] = "You are immune to nether forces.";
	}
        else if (p_ptr->resist_neth)
	{
		info[i++] = "You are resistant to nether forces.";
	}
	if (p_ptr->resist_fear)
	{
		info[i++] = "You are completely fearless.";
	}
	if (p_ptr->resist_blind)
	{
		info[i++] = "Your eyes are resistant to blindness.";
	}
        if (p_ptr->resist_continuum)
	{
                info[i++] = "The space-time continuum cannot be disrupted near you.";
	}

	if (p_ptr->sustain_str)
	{
		info[i++] = "Your strength is sustained.";
	}
	if (p_ptr->sustain_int)
	{
		info[i++] = "Your intelligence is sustained.";
	}
	if (p_ptr->sustain_wis)
	{
		info[i++] = "Your wisdom is sustained.";
	}
	if (p_ptr->sustain_con)
	{
		info[i++] = "Your constitution is sustained.";
	}
	if (p_ptr->sustain_dex)
	{
		info[i++] = "Your dexterity is sustained.";
	}
	if (p_ptr->sustain_chr)
	{
		info[i++] = "Your charisma is sustained.";
	}
        if (p_ptr->black_breath)
        {
                info[i++] = "You suffer from Black Breath.";
        }

	if (f1 & (TR1_STR))
	{
		info[i++] = "Your strength is affected by your equipment.";
	}
	if (f1 & (TR1_INT))
	{
		info[i++] = "Your intelligence is affected by your equipment.";
	}
	if (f1 & (TR1_WIS))
	{
		info[i++] = "Your wisdom is affected by your equipment.";
	}
	if (f1 & (TR1_DEX))
	{
		info[i++] = "Your dexterity is affected by your equipment.";
	}
	if (f1 & (TR1_CON))
	{
		info[i++] = "Your constitution is affected by your equipment.";
	}
	if (f1 & (TR1_CHR))
	{
		info[i++] = "Your charisma is affected by your equipment.";
	}

	if (f1 & (TR1_STEALTH))
	{
		info[i++] = "Your stealth is affected by your equipment.";
	}
	if (f1 & (TR1_SEARCH))
	{
		info[i++] = "Your searching ability is affected by your equipment.";
	}
	if (f1 & (TR1_INFRA))
	{
		info[i++] = "Your infravision is affected by your equipment.";
	}
	if (f1 & (TR1_TUNNEL))
	{
		info[i++] = "Your digging ability is affected by your equipment.";
	}
	if (f1 & (TR1_SPEED))
	{
		info[i++] = "Your speed is affected by your equipment.";
	}
	if (f1 & (TR1_BLOWS))
	{
		info[i++] = "Your attack speed is affected by your equipment.";
	}
        if (f5 & (TR5_CRIT))
	{
                info[i++] = "Your ability to score critical hits is affected by your equipment.";
	}


	/* Access the current weapon */
	o_ptr = &inventory[INVEN_WIELD];

	/* Analyze the weapon */
	if (o_ptr->k_idx)
	{
                object_flags(o_ptr, &f1, &f2, &f3, &f4, &f5, &esp);

		/* Indicate Blessing */
		if (f3 & (TR3_BLESSED))
		{
			info[i++] = "Your weapon has been blessed by the gods.";
		}

		if (f1 & (TR1_CHAOTIC))
		{
                        info[i++] = "Your weapon is branded with the Sign of Chaos.";
		}

		/* Hack */
		if (f1 & (TR1_IMPACT))
		{
			info[i++] = "The impact of your weapon can cause earthquakes.";
		}

		if (f1 & (TR1_VORPAL))
		{
			info[i++] = "Your weapon is very sharp.";
		}

		if (f1 & (TR1_VAMPIRIC))
		{
			info[i++] = "Your weapon drains life from your foes.";
		}

		/* Special "Attack Bonuses" */
		if (f1 & (TR1_BRAND_ACID))
		{
			info[i++] = "Your weapon melts your foes.";
		}
		if (f1 & (TR1_BRAND_ELEC))
		{
			info[i++] = "Your weapon shocks your foes.";
		}
		if (f1 & (TR1_BRAND_FIRE))
		{
			info[i++] = "Your weapon burns your foes.";
		}
		if (f1 & (TR1_BRAND_COLD))
		{
			info[i++] = "Your weapon freezes your foes.";
		}
		if (f1 & (TR1_BRAND_POIS))
		{
			info[i++] = "Your weapon poisons your foes.";
		}

		/* Special "slay" flags */
		if (f1 & (TR1_SLAY_ANIMAL))
		{
			info[i++] = "Your weapon strikes at animals with extra force.";
		}
		if (f1 & (TR1_SLAY_EVIL))
		{
			info[i++] = "Your weapon strikes at evil with extra force.";
		}
		if (f1 & (TR1_SLAY_UNDEAD))
		{
			info[i++] = "Your weapon strikes at undead with holy wrath.";
		}
		if (f1 & (TR1_SLAY_DEMON))
		{
			info[i++] = "Your weapon strikes at demons with holy wrath.";
		}
		if (f1 & (TR1_SLAY_ORC))
		{
			info[i++] = "Your weapon is especially deadly against orcs.";
		}
		if (f1 & (TR1_SLAY_TROLL))
		{
			info[i++] = "Your weapon is especially deadly against trolls.";
		}
		if (f1 & (TR1_SLAY_GIANT))
		{
			info[i++] = "Your weapon is especially deadly against giants.";
		}
		if (f1 & (TR1_SLAY_DRAGON))
		{
			info[i++] = "Your weapon is especially deadly against dragons.";
		}

		/* Special "kill" flags */
		if (f1 & (TR1_KILL_DRAGON))
		{
			info[i++] = "Your weapon is a great bane of dragons.";
		}
		/* Special "kill" flags */
                if (f5 & (TR5_KILL_DEMON))
		{
                        info[i++] = "Your weapon is a great bane of demons.";
		}
		/* Special "kill" flags */
                if (f5 & (TR5_KILL_UNDEAD))
		{
                        info[i++] = "Your weapon is a great bane of undeads.";
		}
	}

        /* Print on screen or in a file ? */
        if (fff == NULL)
        {
                /* Save the screen */
                character_icky = TRUE;
                Term_save();

                /* Erase the screen */
                for (k = 1; k < 24; k++) prt("", k, 13);

                /* Label the information */
                prt("     Your Attributes:", 1, 15);

                /* We will print on top of the map (column 13) */
                for (k = 2, j = 0; j < i; j++)
                {
                        /* Show the info */
                        prt(info[j], k++, 15);

                        /* Every 20 entries (lines 2 to 21), start over */
                        if ((k == 22) && (j+1 < i))
                        {
                                prt("-- more --", k, 15);
                                inkey();
                                for (; k > 2; k--) prt("", k, 15);
                        }
                }

                /* Pause */
                prt("[Press any key to continue]", k, 13);
                inkey();

                /* Restore the screen */
                Term_load();
                character_icky = FALSE;
        }
        else
        {
                /* Label the information */
                fprintf(fff, "     Your Attributes:\n");

                /* We will print on top of the map (column 13) */
                for (j = 0; j < i; j ++)
                {
                        /* Show the info */
                        fprintf(fff, "%s\n", info[j]);
                }
        }
}


static int report_magics_aux(int dur)
{
    if (dur <= 5)
    {
        return 0;
    }
    else if (dur <= 10)
    {
        return 1;
    }
    else if (dur <= 20)
    {
        return 2;
    }
    else if (dur <= 50)
    {
        return 3;
    }
    else if (dur <= 100)
    {
        return 4;
    }
    else if (dur <= 200)
    {
        return 5;
    }
    else
    {
        return 6;
    }
}

static cptr report_magic_durations[] =
{
	"for a short time",
	"for a little while",
	"for a while",
	"for a long while",
	"for a long time",
	"for a very long time",
	"for an incredibly long time",
	"until you hit a monster"
};


void report_magics(void)
{
	int             i = 0, j, k;
	
	char Dummy[80];
	
	cptr    info[128];
	int     info2[128];
	
	if (p_ptr->blind)
	{
		info2[i]  = report_magics_aux(p_ptr->blind);
		info[i++] = "You cannot see";
	}
	if (p_ptr->confused)
	{
		info2[i]  = report_magics_aux(p_ptr->confused);
		info[i++] = "You are confused";
	}
	if (p_ptr->afraid)
	{
		info2[i]  = report_magics_aux(p_ptr->afraid);
		info[i++] = "You are terrified";
	}
	if (p_ptr->poisoned)
	{
		info2[i]  = report_magics_aux(p_ptr->poisoned);
		info[i++] = "You are poisoned";
	}
	if (p_ptr->image)
	{
		info2[i]  = report_magics_aux(p_ptr->image);
		info[i++] = "You are hallucinating";
	}
	
	if (p_ptr->blessed)
	{
		info2[i]  = report_magics_aux(p_ptr->blessed);
		info[i++] = "You feel rightous";
	}
	if (p_ptr->hero)
	{
		info2[i]  = report_magics_aux(p_ptr->hero);
		info[i++] = "You feel heroic";
	}
	if (p_ptr->shero)
	{
		info2[i]  = report_magics_aux(p_ptr->shero);
		info[i++] = "You are in a battle rage";
	}
	if (p_ptr->protevil)
	{
		info2[i]  = report_magics_aux(p_ptr->protevil);
		info[i++] = "You are protected from evil";
	}
	if (p_ptr->shield)
	{
		info2[i]  = report_magics_aux(p_ptr->shield);
		info[i++] = "You are protected by a mystic shield";
	}
	if (p_ptr->invuln)
	{
		info2[i]  = report_magics_aux(p_ptr->invuln);
		info[i++] = "You are invulnerable";
	}
        if (p_ptr->tim_wraith)
	{
                info2[i]  = report_magics_aux(p_ptr->tim_wraith);
		info[i++] = "You are incorporeal";
	}
	if (p_ptr->confusing)
	{
		info2[i]  = 7;
		info[i++] = "Your hands are glowing dull red.";
	}
	if (p_ptr->word_recall)
	{
		info2[i]  = report_magics_aux(p_ptr->word_recall);
		info[i++] = "You waiting to be recalled";
	}
	if (p_ptr->oppose_acid)
	{
		info2[i]  = report_magics_aux(p_ptr->oppose_acid);
		info[i++] = "You are resistant to acid";
	}
	if (p_ptr->oppose_elec)
	{
		info2[i]  = report_magics_aux(p_ptr->oppose_elec);
		info[i++] = "You are resistant to lightning";
	}
	if (p_ptr->oppose_fire)
	{
		info2[i]  = report_magics_aux(p_ptr->oppose_fire);
		info[i++] = "You are resistant to fire";
	}
	if (p_ptr->oppose_cold)
	{
		info2[i]  = report_magics_aux(p_ptr->oppose_cold);
		info[i++] = "You are resistant to cold";
	}
	if (p_ptr->oppose_pois)
	{
		info2[i]  = report_magics_aux(p_ptr->oppose_pois);
		info[i++] = "You are resistant to poison";
	}

	/* Save the screen */
        character_icky = TRUE;
	Term_save();
	
	/* Erase the screen */
	for (k = 1; k < 24; k++) prt("", k, 13);
	
	/* Label the information */
	prt("     Your Current Magic:", 1, 15);
	
	/* We will print on top of the map (column 13) */
	for (k = 2, j = 0; j < i; j++)
	{
		/* Show the info */
		sprintf( Dummy, "%s %s.", info[j],
			report_magic_durations[info2[j]] );
		prt(Dummy, k++, 15);
		
		/* Every 20 entries (lines 2 to 21), start over */
		if ((k == 22) && (j + 1 < i))
		{
			prt("-- more --", k, 15);
			inkey();
			for (; k > 2; k--) prt("", k, 15);
		}
	}
	
	/* Pause */
	prt("[Press any key to continue]", k, 13);
	inkey();

	/* Restore the screen */
	Term_load();
	character_icky = FALSE;
}



/*
 * Forget everything
 */
bool lose_all_info(void)
{
	int i;

	/* Forget info about objects */
	for (i = 0; i < INVEN_TOTAL; i++)
	{
		object_type *o_ptr = &inventory[i];

		/* Skip non-objects */
		if (!o_ptr->k_idx) continue;

		/* Allow "protection" by the MENTAL flag */
		if (o_ptr->ident & (IDENT_MENTAL)) continue;

		/* Remove sensing */
		o_ptr->sense = SENSE_NONE;

		/* Hack -- Clear the "empty" flag */
		o_ptr->ident &= ~(IDENT_EMPTY);

		/* Hack -- Clear the "known" flag */
		o_ptr->ident &= ~(IDENT_KNOWN);

		/* Hack -- Clear the "felt" flag */
		o_ptr->ident &= ~(IDENT_SENSE);
	}

	/* Recalculate bonuses */
	p_ptr->update |= (PU_BONUS);

	/* Combine / Reorder the pack (later) */
	p_ptr->notice |= (PN_COMBINE | PN_REORDER);

	/* Window stuff */
	p_ptr->window |= (PW_INVEN | PW_EQUIP | PW_PLAYER);

	/* Mega-Hack -- Forget the map */
	wiz_dark();

	/* It worked */
	return (TRUE);
}




/*
 * Detect all traps on current panel
 */
bool detect_traps(void)
{
	int             x, y;
	bool            detect = FALSE;
	cave_type       *c_ptr;


	/* Scan the current panel */
	for (y = panel_row_min; y <= panel_row_max; y++)
	{
		for (x = panel_col_min; x <= panel_col_max; x++)
		{
			/* Access the grid */
			c_ptr = &cave[y][x];

			/* Detect invisible traps */
			if (c_ptr->t_idx != 0)
			{
				/* Pick a trap */
				pick_trap(y, x);

				/* Hack -- Memorize */
				c_ptr->info |= (CAVE_MARK);

				/* Redraw */
				lite_spot(y, x);

				/* Obvious */
				detect = TRUE;
			}
		}
	}

	/* Describe */
	if (detect)
	{
		msg_print("You sense the presence of traps!");
	}

	/* Result */
	return (detect);
}



/*
 * Detect all doors on current panel
 */
bool detect_doors(void)
{
	int y, x;

	bool detect = FALSE;

	cave_type *c_ptr;


	/* Scan the panel */
	for (y = panel_row_min; y <= panel_row_max; y++)
	{
		for (x = panel_col_min; x <= panel_col_max; x++)
		{
			c_ptr = &cave[y][x];

			/* Detect secret doors */
			if (c_ptr->feat == FEAT_SECRET)
			{
				/* Pick a door XXX XXX XXX */
				cave_set_feat(y, x, FEAT_DOOR_HEAD + 0x00);
			}

			/* Detect doors */
			if (((c_ptr->feat >= FEAT_DOOR_HEAD) &&
			     (c_ptr->feat <= FEAT_DOOR_HEAD)) ||
			    ((c_ptr->feat == FEAT_OPEN) ||
			     (c_ptr->feat == FEAT_BROKEN)))
			{
				/* Hack -- Memorize */
				c_ptr->info |= (CAVE_MARK);

				/* Redraw */
				lite_spot(y, x);

				/* Obvious */
				detect = TRUE;
			}
		}
	}

	/* Describe */
	if (detect)
	{
		msg_print("You sense the presence of doors!");
	}

	/* Result */
	return (detect);
}


/*
 * Detect all stairs on current panel
 */
bool detect_stairs(void)
{
	int y, x;

	bool detect = FALSE;

	cave_type *c_ptr;


	/* Scan the panel */
	for (y = panel_row_min; y <= panel_row_max; y++)
	{
		for (x = panel_col_min; x <= panel_col_max; x++)
		{
			c_ptr = &cave[y][x];

			/* Detect stairs */
			if ((c_ptr->feat == FEAT_LESS) ||
			    (c_ptr->feat == FEAT_MORE))
			{
				/* Hack -- Memorize */
				c_ptr->info |= (CAVE_MARK);

				/* Redraw */
				lite_spot(y, x);

				/* Obvious */
				detect = TRUE;
			}
		}
	}

	/* Describe */
	if (detect)
	{
		msg_print("You sense the presence of stairs!");
	}

	/* Result */
	return (detect);
}


/*
 * Detect any treasure on the current panel
 */
bool detect_treasure(void)
{
	int y, x;

	bool detect = FALSE;

	cave_type *c_ptr;


	/* Scan the current panel */
	for (y = panel_row_min; y <= panel_row_max; y++)
	{
		for (x = panel_col_min; x <= panel_col_max; x++)
		{
			c_ptr = &cave[y][x];

			/* Notice embedded gold */
			if ((c_ptr->feat == FEAT_MAGMA_H) ||
			    (c_ptr->feat == FEAT_QUARTZ_H))
			{
				/* Expose the gold */
				c_ptr->feat += 0x02;
			}
			else if (c_ptr->feat == FEAT_SANDWALL_H)
			{
				/* Expose the gold */
				c_ptr->feat = FEAT_SANDWALL_K;
			}

			/* Magma/Quartz + Known Gold */
			if ((c_ptr->feat == FEAT_MAGMA_K) ||
			    (c_ptr->feat == FEAT_QUARTZ_K) ||
			    (c_ptr->feat == FEAT_SANDWALL_K))
			{
				/* Hack -- Memorize */
				c_ptr->info |= (CAVE_MARK);

				/* Redraw */
				lite_spot(y, x);

				/* Detect */
				detect = TRUE;
			}
		}
	}

	/* Describe */
	if (detect)
	{
		msg_print("You sense the presence of buried treasure!");
	}



	/* Result */
	return (detect);
}



/*
 * Detect all "gold" objects on the current panel
 */
bool hack_no_detect_message = FALSE;
bool detect_objects_gold(void)
{
	int i, y, x;

	bool detect = FALSE;


	/* Scan objects */
	for (i = 1; i < o_max; i++)
	{
		object_type *o_ptr = &o_list[i];

		/* Skip dead objects */
		if (!o_ptr->k_idx) continue;

		/* Skip held objects */
		if (o_ptr->held_m_idx) continue;

		/* Location */
		y = o_ptr->iy;
		x = o_ptr->ix;

		/* Only detect nearby objects */
		if (!panel_contains(y, x)) continue;
		
		/* Detect "gold" objects */
		if (o_ptr->tval == TV_GOLD)
		{
			/* Hack -- memorize it */
			o_ptr->marked = TRUE;

			/* Redraw */
			lite_spot(y, x);

			/* Detect */
			detect = TRUE;
		}
	}

	/* Describe */
        if (detect && (!hack_no_detect_message))
	{
		msg_print("You sense the presence of treasure!");
	}

	if (detect_monsters_string("$*"))
	{
		detect = TRUE;
	}

	/* Result */
	return (detect);
}


/*
 * Detect all "normal" objects on the current panel
 */
bool detect_objects_normal(void)
{
	int i, y, x;

	bool detect = FALSE;


	/* Scan objects */
	for (i = 1; i < o_max; i++)
	{
		object_type *o_ptr = &o_list[i];

		/* Skip dead objects */
		if (!o_ptr->k_idx) continue;

		/* Skip held objects */
		if (o_ptr->held_m_idx) continue;

		/* Location */
		y = o_ptr->iy;
		x = o_ptr->ix;

		/* Only detect nearby objects */
		if (!panel_contains(y, x)) continue;
		
		/* Detect "real" objects */
		if (o_ptr->tval != TV_GOLD)
		{
			/* Hack -- memorize it */
			o_ptr->marked = TRUE;

			/* Redraw */
			lite_spot(y, x);

			/* Detect */
			detect = TRUE;
		}
	}

	/* Describe */
        if (detect && (!hack_no_detect_message))
	{
		msg_print("You sense the presence of objects!");
	}

	if (detect_monsters_string("!=?|"))
	{
		detect = TRUE;
	}

	/* Result */
	return (detect);
}


/*
 * Detect all "magic" objects on the current panel.
 *
 * This will light up all spaces with "magic" items, including artifacts,
 * ego-items, potions, scrolls, books, rods, wands, staves, amulets, rings,
 * and "enchanted" items of the "good" variety.
 *
 * It can probably be argued that this function is now too powerful.
 */
bool detect_objects_magic(void)
{
	int i, y, x, tv;

	bool detect = FALSE;


	/* Scan all objects */
	for (i = 1; i < o_max; i++)
	{
		object_type *o_ptr = &o_list[i];
		
		/* Skip dead objects */
		if (!o_ptr->k_idx) continue;

		/* Skip held objects */
		if (o_ptr->held_m_idx) continue;

		/* Location */
		y = o_ptr->iy;
		x = o_ptr->ix;

		/* Only detect nearby objects */
		if (!panel_contains(y,x)) continue;
		
		/* Examine the tval */
		tv = o_ptr->tval;

		/* Artifacts, misc magic items, or enchanted wearables */
		if (artifact_p(o_ptr) || ego_item_p(o_ptr) || o_ptr->art_name ||
                    (tv == TV_AMULET) || (tv == TV_RING) || (tv == TV_BATERIE) ||
                    (tv == TV_STAFF) || (tv == TV_WAND) || (tv == TV_ROD) || (tv == TV_ROD_MAIN) ||
                    (tv == TV_SCROLL) || (tv == TV_POTION) || (tv == TV_POTION2) ||
                    (tv == TV_VALARIN_BOOK) || (tv == TV_MAGERY_BOOK) ||
                    (tv == TV_SHADOW_BOOK) || (tv == TV_SPIRIT_BOOK) ||
                    (tv == TV_CRUSADE_BOOK) || (tv == TV_SYMBIOTIC_BOOK) || 
		    (tv == TV_MUSIC_BOOK) || ((o_ptr->to_a > 0) || 
		    (o_ptr->to_h + o_ptr->to_d > 0)))
		{
			/* Memorize the item */
			o_ptr->marked = TRUE;

			/* Redraw */
			lite_spot(y, x);

			/* Detect */
			detect = TRUE;
		}
	}

	/* Describe */
	if (detect)
	{
		msg_print("You sense the presence of magic objects!");
	}

	/* Return result */
	return (detect);
}


/*
 * Detect all "normal" monsters on the current panel
 */
bool detect_monsters_normal(void)
{
	int i, y, x;

	bool flag = FALSE;


	/* Scan monsters */
	for (i = 1; i < m_max; i++)
	{
		monster_type *m_ptr = &m_list[i];
                monster_race *r_ptr = race_inf(m_ptr);

		/* Skip dead monsters */
		if (!m_ptr->r_idx) continue;

		/* Location */
		y = m_ptr->fy;
		x = m_ptr->fx;

		/* Only detect nearby monsters */
		if (!panel_contains(y, x)) continue;
		
		/* Detect all non-invisible monsters */
		if ((!(r_ptr->flags2 & (RF2_INVISIBLE))) ||
		    p_ptr->see_inv || p_ptr->tim_invis)
		{
			/* Repair visibility later */
			repair_monsters = TRUE;

			/* Hack -- Detect monster */
			m_ptr->mflag |= (MFLAG_MARK | MFLAG_SHOW);

			/* Hack -- See monster */
			m_ptr->ml = TRUE;
			
			/* Redraw */
			lite_spot(y, x);

			/* Detect */
			flag = TRUE;
		}
	}

	/* Describe */
	if (flag)
	{
		/* Describe result */
		msg_print("You sense the presence of monsters!");
	}

	/* Result */
	return (flag);
}


/*
 * Detect all "invisible" monsters on current panel
 */
bool detect_monsters_invis(void)
{
	int i, y, x;
	bool flag = FALSE;

	/* Scan monsters */
	for (i = 1; i < m_max; i++)
	{
		monster_type *m_ptr = &m_list[i];
                monster_race *r_ptr = race_inf(m_ptr);

		/* Skip dead monsters */
		if (!m_ptr->r_idx) continue;

		/* Location */
		y = m_ptr->fy;
		x = m_ptr->fx;

		/* Only detect nearby monsters */
		if (!panel_contains(y, x)) continue;
		
		/* Detect invisible monsters */
		if (r_ptr->flags2 & (RF2_INVISIBLE))
		{
			/* Take note that they are invisible */
			r_ptr->r_flags2 |= (RF2_INVISIBLE);

			/* Update monster recall window */
			if (monster_race_idx == m_ptr->r_idx)
			{
				/* Window stuff */
				p_ptr->window |= (PW_MONSTER);
			}

			/* Repair visibility later */
			repair_monsters = TRUE;

			/* Hack -- Detect monster */
			m_ptr->mflag |= (MFLAG_MARK | MFLAG_SHOW);

			/* Hack -- See monster */
			m_ptr->ml = TRUE;
			
			/* Redraw */
			lite_spot(y, x);

			/* Detect */
			flag = TRUE;
		}
	}

	/* Describe */
	if (flag)
	{
		/* Describe result */
		msg_print("You sense the presence of invisible creatures!");
	}

	/* Result */
	return (flag);
}



/*
 * Detect all "evil" monsters on current panel
 */
bool detect_monsters_evil(void)
{
	int i, y, x;
	bool flag = FALSE;


	/* Scan monsters */
	for (i = 1; i < m_max; i++)
	{
		monster_type *m_ptr = &m_list[i];
                monster_race *r_ptr = race_inf(m_ptr);

		/* Skip dead monsters */
		if (!m_ptr->r_idx) continue;

		/* Location */
		y = m_ptr->fy;
		x = m_ptr->fx;

		/* Only detect nearby monsters */
		if (!panel_contains(y, x)) continue;
		
		/* Detect evil monsters */
		if (r_ptr->flags3 & (RF3_EVIL))
		{
			/* Take note that they are evil */
			r_ptr->r_flags3 |= (RF3_EVIL);

			/* Update monster recall window */
			if (monster_race_idx == m_ptr->r_idx)
			{
				/* Window stuff */
				p_ptr->window |= (PW_MONSTER);
			}

			/* Repair visibility later */
			repair_monsters = TRUE;

			/* Hack -- Detect monster */
			m_ptr->mflag |= (MFLAG_MARK | MFLAG_SHOW);

			/* Hack -- See monster */
			m_ptr->ml = TRUE;
			
			/* Redraw */
			lite_spot(y, x);

			/* Detect */
			flag = TRUE;
		}
	}

	/* Describe */
	if (flag)
	{
		/* Describe result */
		msg_print("You sense the presence of evil creatures!");
	}

	/* Result */
	return (flag);
}




/*
 * Detect all (string) monsters on current panel
 */
bool detect_monsters_string(cptr Match)
{
	int i, y, x;
	bool flag = FALSE;


	/* Scan monsters */
	for (i = 1; i < m_max; i++)
	{
		monster_type *m_ptr = &m_list[i];
                monster_race *r_ptr = race_inf(m_ptr);

		/* Skip dead monsters */
		if (!m_ptr->r_idx) continue;

		/* Location */
		y = m_ptr->fy;
		x = m_ptr->fx;

		/* Only detect nearby monsters */
		if (!panel_contains(y, x)) continue;
		
		/* Detect evil monsters */
		if (strchr(Match, r_ptr->d_char))
		{

			/* Update monster recall window */
			if (monster_race_idx == m_ptr->r_idx)
			{
				/* Window stuff */
				p_ptr->window |= (PW_MONSTER);
			}

			/* Repair visibility later */
			repair_monsters = TRUE;

			/* Hack -- Detect monster */
			m_ptr->mflag |= (MFLAG_MARK | MFLAG_SHOW);

			/* Hack -- See monster */
			m_ptr->ml = TRUE;
			
			/* Redraw */
			lite_spot(y, x);

			/* Detect */
			flag = TRUE;
		}
	}

	/* Describe */
	if (flag)
	{
		/* Describe result */
		msg_print("You sense the presence of monsters!");
	}

	/* Result */
	return (flag);
}


/*
 * A "generic" detect monsters routine, tagged to flags3
 */
bool detect_monsters_xxx(u32b match_flag)
{
	int  i, y, x;
	bool flag = FALSE;
	cptr desc_monsters = "weird monsters";


	/* Scan monsters */
	for (i = 1; i < m_max; i++)
	{
		monster_type *m_ptr = &m_list[i];
                monster_race *r_ptr = race_inf(m_ptr);

		/* Skip dead monsters */
		if (!m_ptr->r_idx) continue;

		/* Location */
		y = m_ptr->fy;
		x = m_ptr->fx;

		/* Only detect nearby monsters */
		if (!panel_contains(y, x)) continue;
		
		/* Detect evil monsters */
		if (r_ptr->flags3 & (match_flag))
		{
			/* Take note that they are something */
			r_ptr->r_flags3 |= (match_flag);

			/* Update monster recall window */
			if (monster_race_idx == m_ptr->r_idx)
			{
				/* Window stuff */
				p_ptr->window |= (PW_MONSTER);
			}

			/* Repair visibility later */
			repair_monsters = TRUE;

			/* Hack -- Detect monster */
			m_ptr->mflag |= (MFLAG_MARK | MFLAG_SHOW);

			/* Hack -- See monster */
			m_ptr->ml = TRUE;
			
			/* Redraw */
			lite_spot(y, x);

			/* Detect */
			flag = TRUE;
		}
	}

	/* Describe */
	if (flag)
	{
		switch (match_flag)
		{
			case RF3_DEMON:
				desc_monsters = "demons";
				break;
			case RF3_UNDEAD:
				desc_monsters = "the undead";
				break;
			case RF3_GOOD:
				desc_monsters = "good";
				break;
		}

		/* Describe result */
		msg_format("You sense the presence of %s!", desc_monsters);
		msg_print(NULL);
	}

	/* Result */
	return (flag);
}

/* Detect good monsters */
bool detect_monsters_good(void)
{
	return(detect_monsters_xxx(RF3_GOOD));
}


/*
 * Detect everything
 */
bool detect_all(void)
{
	bool detect = FALSE;

	/* Detect everything */
	if (detect_traps()) detect = TRUE;
	if (detect_doors()) detect = TRUE;
	if (detect_stairs()) detect = TRUE;
	if (detect_treasure()) detect = TRUE;
	if (detect_objects_gold()) detect = TRUE;
	if (detect_objects_normal()) detect = TRUE;
	if (detect_monsters_invis()) detect = TRUE;
	if (detect_monsters_normal()) detect = TRUE;
	
	/* Result */
	return (detect);
}



/*
 * Create stairs at the player location
 */
void stair_creation(void)
{
	/* XXX XXX XXX */
	if (!cave_valid_bold(py, px))
	{
		msg_print("The object resists the spell.");
		return;
	}

        if (dungeon_flags1 & LF1_SPECIAL)
	{
		msg_print("No stair creation on special levels...");
		return;
	}

	/* XXX XXX XXX */
	delete_object(py, px);

	/* Create a staircase */
	if (p_ptr->inside_arena || p_ptr->inside_quest)
	{
		/* arena or quest */
		msg_print("There is no effect!");
	}
	else if (!dun_level)
	{
		/* Town/wilderness */
		cave_set_feat(py, px, FEAT_MORE);
	}
	else if (is_quest(dun_level) || (dun_level >= MAX_DEPTH-1))
	{
		/* Quest level */
		cave_set_feat(py, px, FEAT_LESS);
	}
	else if (rand_int(100) < 50)
	{
		cave_set_feat(py, px, FEAT_MORE);
	}
	else
	{
		cave_set_feat(py, px, FEAT_LESS);
	}
}




/*
 * Hook to specify "weapon"
 */
static bool item_tester_hook_weapon(object_type *o_ptr)
{
	switch (o_ptr->tval)
	{
                case TV_MSTAFF:
                case TV_BOOMERANG:
                case TV_SWORD:
                case TV_AXE:
		case TV_HAFTED:
		case TV_POLEARM:
		case TV_DIGGING:
		case TV_BOW:
		case TV_BOLT:
		case TV_ARROW:
		case TV_SHOT:
		{
			return (TRUE);
		}
	}

	return (FALSE);
}


/*
 * Hook to specify "armour"
 */
bool item_tester_hook_armour(object_type *o_ptr)
{
	switch (o_ptr->tval)
	{
		case TV_DRAG_ARMOR:
		case TV_HARD_ARMOR:
		case TV_SOFT_ARMOR:
		case TV_SHIELD:
		case TV_CLOAK:
		case TV_CROWN:
		case TV_HELM:
		case TV_BOOTS:
		case TV_GLOVES:
		{
			return (TRUE);
		}
	}

	return (FALSE);
}


/*
 * Check if an object is weapon or armour (but not arrow, bolt, or shot)
 */
bool item_tester_hook_weapon_armour(object_type *o_ptr)
{
	return(item_tester_hook_weapon(o_ptr) ||
	       item_tester_hook_armour(o_ptr));
}

/*
 * Check if an object is artifactable
 */
bool item_tester_hook_artifactable(object_type *o_ptr)
{
	return(item_tester_hook_weapon(o_ptr) ||
               item_tester_hook_armour(o_ptr) ||
               (o_ptr->tval == TV_RING) || (o_ptr->tval == TV_AMULET));
}


/*
 * Enchants a plus onto an item. -RAK-
 *
 * Revamped!  Now takes item pointer, number of times to try enchanting,
 * and a flag of what to try enchanting.  Artifacts resist enchantment
 * some of the time, and successful enchantment to at least +0 might
 * break a curse on the item. -CFT-
 *
 * Note that an item can technically be enchanted all the way to +15 if
 * you wait a very, very, long time.  Going from +9 to +10 only works
 * about 5% of the time, and from +10 to +11 only about 1% of the time.
 *
 * Note that this function can now be used on "piles" of items, and
 * the larger the pile, the lower the chance of success.
 */
bool enchant(object_type *o_ptr, int n, int eflag)
{
	int     i, chance, prob;
	bool    res = FALSE;
	bool    a = (artifact_p(o_ptr) || o_ptr->art_name);
        u32b    f1, f2, f3, f4, f5, esp;


	/* Extract the flags */
        object_flags(o_ptr, &f1, &f2, &f3, &f4, &f5, &esp);

	/* Large piles resist enchantment */
	prob = o_ptr->number * 100;

	/* Missiles are easy to enchant */
	if ((o_ptr->tval == TV_BOLT) ||
	    (o_ptr->tval == TV_ARROW) ||
	    (o_ptr->tval == TV_SHOT))
	{
		prob = prob / 20;
	}

	/* Try "n" times */
	for (i=0; i<n; i++)
	{
		/* Hack -- Roll for pile resistance */
		if (rand_int(prob) >= 100) continue;

		/* Enchant to hit */
		if (eflag & (ENCH_TOHIT))
		{
			if (o_ptr->to_h < 0) chance = 0;
			else if (o_ptr->to_h > 15) chance = 1000;
			else chance = enchant_table[o_ptr->to_h];

			if ((randint(1000) > chance) && (!a || (rand_int(100) < 50)))
			{
				o_ptr->to_h++;
				res = TRUE;

				/* only when you get it above -1 -CFT */
				if (cursed_p(o_ptr) &&
				    (!(f3 & (TR3_PERMA_CURSE))) &&
				    (o_ptr->to_h >= 0) && (rand_int(100) < 25))
				{
					msg_print("The curse is broken!");
					o_ptr->ident &= ~(IDENT_CURSED);
					o_ptr->ident |= (IDENT_SENSE);

					if (o_ptr->art_flags3 & (TR3_CURSED))
					    o_ptr->art_flags3 &= ~(TR3_CURSED);
					if (o_ptr->art_flags3 & (TR3_HEAVY_CURSE))
					    o_ptr->art_flags3 &= ~(TR3_HEAVY_CURSE);

                                        o_ptr->sense = SENSE_UNCURSED;
				}
			}
		}

		/* Enchant to damage */
		if (eflag & (ENCH_TODAM))
		{
			if (o_ptr->to_d < 0) chance = 0;
			else if (o_ptr->to_d > 15) chance = 1000;
			else chance = enchant_table[o_ptr->to_d];

			if ((randint(1000) > chance) && (!a || (rand_int(100) < 50)))
			{
				o_ptr->to_d++;
				res = TRUE;

				/* only when you get it above -1 -CFT */
				if (cursed_p(o_ptr) &&
				    (!(f3 & (TR3_PERMA_CURSE))) &&
				    (o_ptr->to_d >= 0) && (rand_int(100) < 25))
				{
					msg_print("The curse is broken!");
					o_ptr->ident &= ~(IDENT_CURSED);
					o_ptr->ident |= (IDENT_SENSE);

					if (o_ptr->art_flags3 & (TR3_CURSED))
					    o_ptr->art_flags3 &= ~(TR3_CURSED);
					if (o_ptr->art_flags3 & (TR3_HEAVY_CURSE))
					    o_ptr->art_flags3 &= ~(TR3_HEAVY_CURSE);

                                        o_ptr->sense = SENSE_UNCURSED;
				}
			}
		}


		/* Enchant to damage */
                if (eflag & (ENCH_PVAL))
		{
                        if (o_ptr->pval < 0) chance = 0;
                        else if (o_ptr->pval > 8) chance = 1000;
                        else chance = enchant_table[o_ptr->pval];

			if ((randint(1000) > chance) && (!a || (rand_int(100) < 50)))
			{
                                o_ptr->pval++;
				res = TRUE;

				/* only when you get it above -1 -CFT */
				if (cursed_p(o_ptr) &&
				    (!(f3 & (TR3_PERMA_CURSE))) &&
                                    (o_ptr->pval >= 0) && (rand_int(100) < 25))
				{
					msg_print("The curse is broken!");
					o_ptr->ident &= ~(IDENT_CURSED);
					o_ptr->ident |= (IDENT_SENSE);

					if (o_ptr->art_flags3 & (TR3_CURSED))
					    o_ptr->art_flags3 &= ~(TR3_CURSED);
					if (o_ptr->art_flags3 & (TR3_HEAVY_CURSE))
					    o_ptr->art_flags3 &= ~(TR3_HEAVY_CURSE);

                                        o_ptr->sense = SENSE_UNCURSED;
				}
			}
		}

		/* Enchant to armor class */
		if (eflag & (ENCH_TOAC))
		{
			if (o_ptr->to_a < 0) chance = 0;
			else if (o_ptr->to_a > 15) chance = 1000;
			else chance = enchant_table[o_ptr->to_a];

			if ((randint(1000) > chance) && (!a || (rand_int(100) < 50)))
			{
				o_ptr->to_a++;
				res = TRUE;

				/* only when you get it above -1 -CFT */
				if (cursed_p(o_ptr) &&
				    (!(f3 & (TR3_PERMA_CURSE))) &&
				    (o_ptr->to_a >= 0) && (rand_int(100) < 25))
				{
					msg_print("The curse is broken!");
					o_ptr->ident &= ~(IDENT_CURSED);
					o_ptr->ident |= (IDENT_SENSE);

					if (o_ptr->art_flags3 & (TR3_CURSED))
					    o_ptr->art_flags3 &= ~(TR3_CURSED);
					if (o_ptr->art_flags3 & (TR3_HEAVY_CURSE))
					    o_ptr->art_flags3 &= ~(TR3_HEAVY_CURSE);

                                        o_ptr->sense = SENSE_UNCURSED;
				}
			}
		}
	}

	/* Failure */
	if (!res) return (FALSE);

	/* Recalculate bonuses */
	p_ptr->update |= (PU_BONUS);

	/* Combine / Reorder the pack (later) */
	p_ptr->notice |= (PN_COMBINE | PN_REORDER);

	/* Window stuff */
	p_ptr->window |= (PW_INVEN | PW_EQUIP | PW_PLAYER);

	/* Success */
	return (TRUE);
}



/*
 * Enchant an item (in the inventory or on the floor)
 * Note that "num_ac" requires armour, else weapon
 * Returns TRUE if attempted, FALSE if cancelled
 */
bool enchant_spell(int num_hit, int num_dam, int num_ac, int num_pval)
{
	int         item;
	bool        okay = FALSE;
	object_type *o_ptr;
	char        o_name[80];
	cptr        q, s;


	/* Assume enchant weapon */
	item_tester_hook = item_tester_hook_weapon;

	/* Enchant armor if requested */
	if (num_ac) item_tester_hook = item_tester_hook_armour;

	/* Get an item */
	q = "Enchant which item? ";
	s = "You have nothing to enchant.";
	if (!get_item(&item, q, s, (USE_EQUIP | USE_INVEN | USE_FLOOR))) return (FALSE);

	/* Get the item (in the pack) */
	if (item >= 0)
	{
		o_ptr = &inventory[item];
	}

	/* Get the item (on the floor) */
	else
	{
		o_ptr = &o_list[0 - item];
	}


	/* Description */
	object_desc(o_name, o_ptr, FALSE, 0);

	/* Describe */
	msg_format("%s %s glow%s brightly!",
		   ((item >= 0) ? "Your" : "The"), o_name,
		   ((o_ptr->number > 1) ? "" : "s"));

	/* Enchant */
	if (enchant(o_ptr, num_hit, ENCH_TOHIT)) okay = TRUE;
	if (enchant(o_ptr, num_dam, ENCH_TODAM)) okay = TRUE;
        if (enchant(o_ptr, num_ac, ENCH_TOAC)) okay = TRUE;
        if (enchant(o_ptr, num_pval, ENCH_PVAL)) okay = TRUE;

	/* Failure */
	if (!okay)
	{
		/* Flush */
		if (flush_failure) flush();

		/* Message */
		msg_print("The enchantment failed.");
	}

	/* Something happened */
	return (TRUE);
}

void curse_artifact(object_type * o_ptr)
{
	if (o_ptr->pval) o_ptr->pval = 0 - ((o_ptr->pval) + randint(4));
	if (o_ptr->to_a) o_ptr->to_a = 0 - ((o_ptr->to_a) + randint(4));
	if (o_ptr->to_h) o_ptr->to_h = 0 - ((o_ptr->to_h) + randint(4));
	if (o_ptr->to_d) o_ptr->to_d = 0 - ((o_ptr->to_d) + randint(4));
	o_ptr->art_flags3 |= ( TR3_HEAVY_CURSE | TR3_CURSED );
	if (randint(4)==1) o_ptr-> art_flags3 |= TR3_PERMA_CURSE;
	if (randint(3)==1) o_ptr-> art_flags3 |= TR3_TY_CURSE;
	if (randint(2)==1) o_ptr-> art_flags3 |= TR3_AGGRAVATE;
	if (randint(3)==1) o_ptr-> art_flags3 |= TR3_DRAIN_EXP;
        if (randint(3)==1) o_ptr-> art_flags4 |= TR4_BLACK_BREATH;
	if (randint(2)==1) o_ptr-> art_flags3 |= TR3_TELEPORT;
	else if (randint(3)==1) o_ptr->art_flags3 |= TR3_NO_TELE;
	o_ptr->ident |= IDENT_CURSED;
}


void random_plus(object_type * o_ptr, bool is_scroll)
{
	int this_type = (o_ptr->tval<TV_BOOTS?23:19);

	if (artifact_bias == BIAS_WARRIOR)
	{
		if (!(o_ptr->art_flags1 & TR1_STR))
		{
			o_ptr->art_flags1 |= TR1_STR;
			if (randint(2)==1) return; /* 50% chance of being a "free" power */
		}

		if (!(o_ptr->art_flags1 & TR1_CON))
		{
			o_ptr->art_flags1 |= TR1_CON;
			if (randint(2)==1) return;
		}

		if (!(o_ptr->art_flags1 & TR1_DEX))
		{
			o_ptr->art_flags1 |= TR1_DEX;
			if (randint(2)==1) return;
		}
	}
	else if (artifact_bias == BIAS_MAGE)
	{
		if (!(o_ptr->art_flags1 & TR1_INT))
		{
			o_ptr->art_flags1 |= TR1_INT;
			if (randint(2)==1) return;
		}
	}
	else if (artifact_bias == BIAS_PRIESTLY)
	{
		if (!(o_ptr->art_flags1 & TR1_WIS))
		{
			o_ptr->art_flags1 |= TR1_WIS;
			if (randint(2)==1) return;
		}
	}
	else if (artifact_bias == BIAS_RANGER)
	{
		if (!(o_ptr->art_flags1 & TR1_CON))
		{
			o_ptr->art_flags1 |= TR1_CON;
			if (randint(2)==1) return; /* 50% chance of being a "free" power */
		}

		if (!(o_ptr->art_flags1 & TR1_DEX))
		{
			o_ptr->art_flags1 |= TR1_DEX;
			if (randint(2)==1) return;
		}

		if (!(o_ptr->art_flags1 & TR1_STR))
		{
			o_ptr->art_flags1 |= TR1_STR;
			if (randint(2)==1) return;
		}
	}
	else if (artifact_bias == BIAS_ROGUE)
	{
		if (!(o_ptr->art_flags1 & TR1_STEALTH))
		{
			o_ptr->art_flags1 |= TR1_STEALTH;
			if (randint(2)==1) return;
		}
		if (!(o_ptr->art_flags1 & TR1_SEARCH))
		{
			o_ptr->art_flags1 |= TR1_SEARCH;
			if (randint(2)==1) return;
		}
	}
	else if (artifact_bias == BIAS_STR)
	{
		if (!(o_ptr->art_flags1 & TR1_STR))
		{
			o_ptr->art_flags1 |= TR1_STR;
			if (randint(2)==1) return;
		}
	}
	else if (artifact_bias == BIAS_WIS)
	{
		if (!(o_ptr->art_flags1 & TR1_WIS))
		{
			o_ptr->art_flags1 |= TR1_WIS;
			if (randint(2)==1) return;
		}
	}
	else if (artifact_bias == BIAS_INT)
	{
		if (!(o_ptr->art_flags1 & TR1_INT))
		{
			o_ptr->art_flags1 |= TR1_INT;
			if (randint(2)==1) return;
		}
	}
	else if (artifact_bias == BIAS_DEX)
	{
		if (!(o_ptr->art_flags1 & TR1_DEX))
		{
			o_ptr->art_flags1 |= TR1_DEX;
			if (randint(2)==1) return;
		}
	}
	else if (artifact_bias == BIAS_CON)
	{
		if (!(o_ptr->art_flags1 & TR1_CON))
		{
			o_ptr->art_flags1 |= TR1_CON;
			if (randint(2)==1) return;
		}
	}
	else if (artifact_bias == BIAS_CHR)
	{
		if (!(o_ptr->art_flags1 & TR1_CHR))
		{
			o_ptr->art_flags1 |= TR1_CHR;
			if (randint(2)==1) return;
		}
	}


	switch (randint(this_type))
	{
	case 1: case 2:
		o_ptr->art_flags1 |= TR1_STR;
		/*  if (is_scroll) msg_print("It makes you feel strong!"); */
		if (!(artifact_bias) && randint(13)!=1)
			artifact_bias = BIAS_STR;
		else if (!(artifact_bias) && randint(7)==1)
			artifact_bias = BIAS_WARRIOR;
		break;
	case 3: case 4:
		o_ptr->art_flags1 |= TR1_INT;
		/*  if (is_scroll) msg_print("It makes you feel smart!"); */
		if (!(artifact_bias) && randint(13)!=1)
			artifact_bias = BIAS_INT;
		else if (!(artifact_bias) && randint(7)==1)
			artifact_bias = BIAS_MAGE;
		break;
	case 5: case 6:
		o_ptr->art_flags1 |= TR1_WIS;
		/*  if (is_scroll) msg_print("It makes you feel wise!"); */
		if (!(artifact_bias) && randint(13)!=1)
			artifact_bias = BIAS_WIS;
		else if (!(artifact_bias) && randint(7)==1)
			artifact_bias = BIAS_PRIESTLY;
		break;
	case 7: case 8:
		o_ptr->art_flags1 |= TR1_DEX;
		/*  if (is_scroll) msg_print("It makes you feel nimble!"); */
		if (!(artifact_bias) && randint(13)!=1)
			artifact_bias = BIAS_DEX;
		else if (!(artifact_bias) && randint(7)==1)
			artifact_bias = BIAS_ROGUE;
		break;
	case 9: case 10:
		o_ptr->art_flags1 |= TR1_CON;
		/*  if (is_scroll) msg_print("It makes you feel healthy!"); */
		if (!(artifact_bias) && randint(13)!=1)
			artifact_bias = BIAS_CON;
		else if (!(artifact_bias) && randint(9)==1)
			artifact_bias = BIAS_RANGER;
		break;
	case 11: case 12:
		o_ptr->art_flags1 |= TR1_CHR;
		/*  if (is_scroll) msg_print("It makes you look great!"); */
		if (!(artifact_bias) && randint(13)!=1)
			artifact_bias = BIAS_CHR;
		break;
	case 13: case 14:
		o_ptr->art_flags1 |= TR1_STEALTH;
		/*  if (is_scroll) msg_print("It looks muffled."); */
		if (!(artifact_bias) && randint(3)==1)
			artifact_bias = BIAS_ROGUE;
		break;
	case 15: case 16:
		o_ptr->art_flags1 |= TR1_SEARCH;
		/*  if (is_scroll) msg_print("It makes you see better."); */
		if (!(artifact_bias) && randint(9)==1)
			artifact_bias = BIAS_RANGER;
		break;
	case 17: case 18:
		o_ptr->art_flags1 |= TR1_INFRA;
		/*  if (is_scroll) msg_print("It makes you see tiny red animals.");*/
		break;
	case 19:
		o_ptr->art_flags1 |= TR1_SPEED;
		/*  if (is_scroll) msg_print("It makes you move faster!"); */
		if (!(artifact_bias) && randint(11)==1)
			artifact_bias = BIAS_ROGUE;
		break;
	case 20: case 21:
		o_ptr->art_flags1 |= TR1_TUNNEL;
		/*  if (is_scroll) msg_print("Gravel flies from it!"); */
		break;
	case 22: case 23:
		if (o_ptr->tval == TV_BOW) random_plus(o_ptr, is_scroll);
		else
		{
			o_ptr->art_flags1 |= TR1_BLOWS;
			/*  if (is_scroll) msg_print("It seems faster!"); */
			if (!(artifact_bias) && randint(11)==1)
				artifact_bias = BIAS_WARRIOR;
		}
		break;
	}
}


void random_resistance (object_type * o_ptr, bool is_scroll, int specific)
{
  if (!specific) /* To avoid a number of possible bugs */
  {
    if (artifact_bias == BIAS_ACID)
    {
	if (!(o_ptr->art_flags2 & TR2_RES_ACID))
	{
	    o_ptr->art_flags2 |= TR2_RES_ACID;
	    if (randint(2)==1) return;
	}
    if (randint(BIAS_LUCK)==1 && !(o_ptr->art_flags2 & TR2_IM_ACID))
	{
	    o_ptr->art_flags2 |= TR2_IM_ACID;
	    if (randint(2)==1) return;
	}
    }
    else if (artifact_bias == BIAS_ELEC)
    {
	if (!(o_ptr->art_flags2 & TR2_RES_ELEC))
	{
	    o_ptr->art_flags2 |= TR2_RES_ELEC;
	    if (randint(2)==1) return;
	}
    if (o_ptr->tval >= TV_CLOAK && o_ptr->tval <= TV_HARD_ARMOR &&
        ! (o_ptr->art_flags3 & TR3_SH_ELEC))
        {
            o_ptr->art_flags2 |= TR3_SH_ELEC;
            if (randint(2)==1) return;
        }
    if (randint(BIAS_LUCK)==1 && !(o_ptr->art_flags2 & TR2_IM_ELEC))
	{
	    o_ptr->art_flags2 |= TR2_IM_ELEC;
	    if (randint(2)==1) return;
	}
    }
    else if (artifact_bias == BIAS_FIRE)
    {
	if (!(o_ptr->art_flags2 & TR2_RES_FIRE))
	{
	    o_ptr->art_flags2 |= TR2_RES_FIRE;
	    if (randint(2)==1) return;
	}
    if (o_ptr->tval >= TV_CLOAK && o_ptr->tval <= TV_HARD_ARMOR &&
        ! (o_ptr->art_flags3 & TR3_SH_FIRE))
        {
            o_ptr->art_flags2 |= TR3_SH_FIRE;
            if (randint(2)==1) return;
        }
    if (randint(BIAS_LUCK)==1 && !(o_ptr->art_flags2 & TR2_IM_FIRE))
	{
	    o_ptr->art_flags2 |= TR2_IM_FIRE;
	    if (randint(2)==1) return;
	}
    }
    else if (artifact_bias == BIAS_COLD)
    {
	if (!(o_ptr->art_flags2 & TR2_RES_COLD))
	{
	    o_ptr->art_flags2 |= TR2_RES_COLD;
	    if (randint(2)==1) return;
	}
    if (randint(BIAS_LUCK)==1 && !(o_ptr->art_flags2 & TR2_IM_COLD))
	{
	    o_ptr->art_flags2 |= TR2_IM_COLD;
	    if (randint(2)==1) return;
	}
    }
    else if (artifact_bias == BIAS_POIS)
    {
	if (!(o_ptr->art_flags2 & TR2_RES_POIS))
	{
	    o_ptr->art_flags2 |= TR2_RES_POIS;
	    if (randint(2)==1) return;
	}
    }
    else if (artifact_bias == BIAS_WARRIOR)
    {
	if (randint(3)!=1 && (!(o_ptr->art_flags2 & TR2_RES_FEAR)))
	{
	    o_ptr->art_flags2 |= TR2_RES_FEAR;
	    if (randint(2)==1) return;
	}
    if ((randint(3)==1) && (!(o_ptr->art_flags3 & TR3_NO_MAGIC)))
    {
        o_ptr->art_flags3 |= TR3_NO_MAGIC;
        if (randint(2)==1) return;
    }
    }
    else if (artifact_bias == BIAS_NECROMANTIC)
    {
	if (!(o_ptr->art_flags2 & TR2_RES_NETHER))
	{
	    o_ptr->art_flags2 |= TR2_RES_NETHER;
	    if (randint(2)==1) return;
	}
	if (!(o_ptr->art_flags2 & TR2_RES_POIS))
	{
	    o_ptr->art_flags2 |= TR2_RES_POIS;
	    if (randint(2)==1) return;
	}
	if (!(o_ptr->art_flags2 & TR2_RES_DARK))
	{
	    o_ptr->art_flags2 |= TR2_RES_DARK;
	    if (randint(2)==1) return;
	}
    }
    else if (artifact_bias == BIAS_CHAOS)
    {
	if (!(o_ptr->art_flags2 & TR2_RES_CHAOS))
	{
	    o_ptr->art_flags2 |= TR2_RES_CHAOS;
	    if (randint(2)==1) return;
	}
	if (!(o_ptr->art_flags2 & TR2_RES_CONF))
	{
	    o_ptr->art_flags2 |= TR2_RES_CONF;
	    if (randint(2)==1) return;
	}
	if (!(o_ptr->art_flags2 & TR2_RES_DISEN))
	{
	    o_ptr->art_flags2 |= TR2_RES_DISEN;
	    if (randint(2)==1) return;
	}
    }
  }

    switch (specific?specific:randint(41))
    {
    case 1:
    if (randint(WEIRD_LUCK)!=1)
        random_resistance(o_ptr, is_scroll, specific);
	else
	{
	o_ptr->art_flags2 |= TR2_IM_ACID;
/*  if (is_scroll) msg_print("It looks totally incorruptible."); */
	if (!(artifact_bias))
	    artifact_bias = BIAS_ACID;
	}
	break;
    case 2:
    if (randint(WEIRD_LUCK)!=1)
	    random_resistance(o_ptr, is_scroll, specific);
	else
	{
	o_ptr->art_flags2 |= TR2_IM_ELEC;
/*  if (is_scroll) msg_print("It looks completely grounded."); */
	if (!(artifact_bias))
	    artifact_bias = BIAS_ELEC;
	}
	break;
    case 3:
    if (randint(WEIRD_LUCK)!=1)
	    random_resistance(o_ptr, is_scroll, specific);
	else
	{
	o_ptr->art_flags2 |= TR2_IM_COLD;
/*  if (is_scroll) msg_print("It feels very warm."); */
	if (!(artifact_bias))
	    artifact_bias = BIAS_COLD;
	}
	break;
    case 4:
    if (randint(WEIRD_LUCK)!=1)
	    random_resistance(o_ptr, is_scroll, specific);
	else
	{
	o_ptr->art_flags2 |= TR2_IM_FIRE;
/*  if (is_scroll) msg_print("It feels very cool."); */
	if (!(artifact_bias))
	    artifact_bias = BIAS_FIRE;
	}
	break;
    case 5: case 6: case 13:
	o_ptr->art_flags2 |= TR2_RES_ACID;
/*  if (is_scroll) msg_print("It makes your stomach rumble."); */
	if (!(artifact_bias))
	    artifact_bias = BIAS_ACID;
	break;
    case 7: case 8: case 14:
	o_ptr->art_flags2 |= TR2_RES_ELEC;
/*  if (is_scroll) msg_print("It makes you feel grounded."); */
    if (!(artifact_bias))
	    artifact_bias = BIAS_ELEC;
	break;
    case 9: case 10: case 15:
	o_ptr->art_flags2 |= TR2_RES_FIRE;
/*  if (is_scroll) msg_print("It makes you feel cool!");*/
	if (!(artifact_bias))
	    artifact_bias = BIAS_FIRE;
	break;
    case 11: case 12: case 16:
	o_ptr->art_flags2 |= TR2_RES_COLD;
/*  if (is_scroll) msg_print("It makes you feel full of hot air!");*/
	if (!(artifact_bias))
	    artifact_bias = BIAS_COLD;
	break;
    case 17: case 18:
	o_ptr->art_flags2 |= TR2_RES_POIS;
/*  if (is_scroll) msg_print("It makes breathing easier for you."); */
	if (!(artifact_bias) && randint(4)!=1)
	    artifact_bias = BIAS_POIS;
	else if (!(artifact_bias) && randint(2)==1)
	    artifact_bias = BIAS_NECROMANTIC;
	else if (!(artifact_bias) && randint(2)==1)
	    artifact_bias = BIAS_ROGUE;
	break;
    case 19: case 20:
	o_ptr->art_flags2 |= TR2_RES_FEAR;
/*  if (is_scroll) msg_print("It makes you feel brave!"); */
	if (!(artifact_bias) && randint(3)==1)
	    artifact_bias = BIAS_WARRIOR;
	break;
    case 21:
	o_ptr->art_flags2 |= TR2_RES_LITE;
/*  if (is_scroll) msg_print("It makes everything look darker.");*/
	break;
    case 22:
	o_ptr->art_flags2 |= TR2_RES_DARK;
/*  if (is_scroll) msg_print("It makes everything look brigher.");*/
	break;
    case 23: case 24:
	o_ptr->art_flags2 |= TR2_RES_BLIND;
/*  if (is_scroll) msg_print("It makes you feel you are wearing glasses.");*/
	break;
    case 25: case 26:
	o_ptr->art_flags2 |= TR2_RES_CONF;
/*  if (is_scroll) msg_print("It makes you feel very determined.");*/
	if (!(artifact_bias) && randint(6)==1)
	    artifact_bias = BIAS_CHAOS;
	break;
    case 27: case 28:
	o_ptr->art_flags2 |= TR2_RES_SOUND;
/*  if (is_scroll) msg_print("It makes you feel deaf!");*/
	break;
    case 29: case 30:
	o_ptr->art_flags2 |= TR2_RES_SHARDS;
/*  if (is_scroll) msg_print("It makes your skin feel thicker.");*/
	break;
    case 31: case 32:
	o_ptr->art_flags2 |= TR2_RES_NETHER;
/*  if (is_scroll) msg_print("It makes you feel like visiting a graveyard!");*/
	if (!(artifact_bias) && randint(3)==1)
	    artifact_bias = BIAS_NECROMANTIC;
	break;
    case 33: case 34:
	o_ptr->art_flags2 |= TR2_RES_NEXUS;
/*  if (is_scroll) msg_print("It makes you feel normal.");*/
	break;
    case 35: case 36:
	o_ptr->art_flags2 |= TR2_RES_CHAOS;
/*  if (is_scroll) msg_print("It makes you feel very firm.");*/
	if (!(artifact_bias) && randint(2)==1)
	    artifact_bias = BIAS_CHAOS;
	break;
    case 37: case 38:
	o_ptr->art_flags2 |= TR2_RES_DISEN;
/*  if (is_scroll) msg_print("It is surrounded by a static feeling.");*/
	break;
    case 39:
    if (o_ptr->tval >= TV_CLOAK && o_ptr->tval <= TV_HARD_ARMOR)
        o_ptr->art_flags3 |= TR3_SH_ELEC;
    else
	    random_resistance(o_ptr, is_scroll, specific);
    if (!(artifact_bias))
	    artifact_bias = BIAS_ELEC;
    break;
    case 40:
    if (o_ptr->tval >= TV_CLOAK && o_ptr->tval <= TV_HARD_ARMOR)
        o_ptr->art_flags3 |= TR3_SH_FIRE;
    else
	    random_resistance(o_ptr, is_scroll, specific);
    if (!(artifact_bias))
        artifact_bias = BIAS_FIRE;
    break;
    case 41:
    if (o_ptr->tval == TV_SHIELD || o_ptr->tval == TV_CLOAK ||
        o_ptr->tval == TV_HELM || o_ptr->tval == TV_HARD_ARMOR)
        o_ptr->art_flags2 |= TR2_REFLECT;
    else
	    random_resistance(o_ptr, is_scroll, specific);
    break;
    }
}

void random_misc (object_type * o_ptr, bool is_scroll)
{

    if (artifact_bias == BIAS_RANGER)
    {
	if (!(o_ptr->art_flags2 & TR2_SUST_CON))
	{
	    o_ptr->art_flags2 |= TR2_SUST_CON;
	    if (randint(2)==1) return;
	}
    }
    else if (artifact_bias == BIAS_STR)
    {
	if (!(o_ptr->art_flags2 & TR2_SUST_STR))
	{
	    o_ptr->art_flags2 |= TR2_SUST_STR;
	    if (randint(2)==1) return;
	}
    }
    else if (artifact_bias == BIAS_WIS)
    {
	if (!(o_ptr->art_flags2 & TR2_SUST_WIS))
	{
	    o_ptr->art_flags2 |= TR2_SUST_WIS;
	    if (randint(2)==1) return;
	}
    }
    else if (artifact_bias == BIAS_INT)
    {
	if (!(o_ptr->art_flags2 & TR2_SUST_INT))
	{
	    o_ptr->art_flags2 |= TR2_SUST_INT;
	    if (randint(2)==1) return;
	}
    }
    else if (artifact_bias == BIAS_DEX)
    {
	if (!(o_ptr->art_flags2 & TR2_SUST_DEX))
	{
	    o_ptr->art_flags2 |= TR2_SUST_DEX;
	    if (randint(2)==1) return;
	}
    }
    else if (artifact_bias == BIAS_CON)
    {
	if (!(o_ptr->art_flags2 & TR2_SUST_CON))
	{
	    o_ptr->art_flags2 |= TR2_SUST_CON;
	    if (randint(2)==1) return;
	}
    }
    else if (artifact_bias == BIAS_CHR)
    {
	if (!(o_ptr->art_flags2 & TR2_SUST_CHR))
	{
	    o_ptr->art_flags2 |= TR2_SUST_CHR;
	    if (randint(2)==1) return;
	}
    }
    else if (artifact_bias == BIAS_CHAOS)
    {
	if (!(o_ptr->art_flags3 & TR3_TELEPORT))
	{
	    o_ptr->art_flags3 |= TR3_TELEPORT;
	    if (randint(2)==1) return;
	}
    }
    else if (artifact_bias == BIAS_FIRE)
    {
        if (!(o_ptr->art_flags3 & TR3_LITE1))
	{
            o_ptr->art_flags3 |= TR3_LITE1; /* Freebie */
	}
    }


    switch (randint(31))
    {
    case 1:
	o_ptr->art_flags2 |= TR2_SUST_STR;
/*  if (is_scroll) msg_print("It makes you feel you cannot become weaker."); */
	if (!artifact_bias)
	    artifact_bias = BIAS_STR;
	break;
    case 2:
	o_ptr->art_flags2 |= TR2_SUST_INT;
/*  if (is_scroll) msg_print("It makes you feel you cannot become more stupid.");*/
	if (!artifact_bias)
	    artifact_bias = BIAS_INT;
	break;
    case 3:
	o_ptr->art_flags2 |= TR2_SUST_WIS;
/*  if (is_scroll) msg_print("It makes you feel you cannot become simpler.");*/
	if (!artifact_bias)
	    artifact_bias = BIAS_WIS;
	break;
    case 4:
	o_ptr->art_flags2 |= TR2_SUST_DEX;
/*  if (is_scroll) msg_print("It makes you feel you cannot become clumsier.");*/
	if (!artifact_bias)
	    artifact_bias = BIAS_DEX;
	break;
    case 5:
	o_ptr->art_flags2 |= TR2_SUST_CON;
/*  if (is_scroll) msg_print("It makes you feel you cannot become less healthy.");*/
	if (!artifact_bias)
	    artifact_bias = BIAS_CON;
	break;
    case 6:
	o_ptr->art_flags2 |= TR2_SUST_CHR;
/*  if (is_scroll) msg_print("It makes you feel you cannot become uglier.");*/
	if (!artifact_bias)
	    artifact_bias = BIAS_CHR;
	break;
    case 7: case 8: case 14:
	o_ptr->art_flags2 |= TR2_FREE_ACT;
/*  if (is_scroll) msg_print("It makes you feel like a young rebel!");*/
	break;
    case 9:
	o_ptr->art_flags2 |= TR2_HOLD_LIFE;
/*  if (is_scroll) msg_print("It makes you feel immortal.");*/
	if (!artifact_bias && (randint(5)==1))
	    artifact_bias = BIAS_PRIESTLY;
	else if (!artifact_bias && (randint(6)==1))
	    artifact_bias = BIAS_NECROMANTIC;
	break;
    case 10: case 11:
        o_ptr->art_flags3 |= TR3_LITE1;
/*  if (is_scroll) msg_print("It starts shining.");*/
	break;
    case 12: case 13:
	o_ptr->art_flags3 |= TR3_FEATHER;
/*  if (is_scroll) msg_print("It feels lighter.");*/
	break;
    case 15: case 16: case 17:
	o_ptr->art_flags3 |= TR3_SEE_INVIS;
/*  if (is_scroll) msg_print("It makes you see the air!");*/
	break;
    case 18:
        o_ptr->art_esp |= 1 << (rand_int(32));
/*  if (is_scroll) msg_print("It makes you hear voices inside your head!");*/
	if (!artifact_bias && (randint(9)==1))
	    artifact_bias = BIAS_MAGE;
	break;
    case 19: case 20:
	o_ptr->art_flags3 |= TR3_SLOW_DIGEST;
/*  if (is_scroll) msg_print("It makes you feel less hungry.");*/
	break;
    case 21: case 22:
	o_ptr->art_flags3 |= TR3_REGEN;
/*  if (is_scroll) msg_print("It looks as good as new.");*/
	break;
    case 23:
	o_ptr->art_flags3 |= TR3_TELEPORT;
/*  if (is_scroll) msg_print("Its position feels uncertain!");*/
	break;
    case 24: case 25: case 26:
	if (o_ptr->tval>=TV_BOOTS) random_misc(o_ptr, is_scroll);
	else
	{
		o_ptr->art_flags3 |= TR3_SHOW_MODS;
		o_ptr->to_a = 4 + (randint(11));
	}
	break;
    case 27: case 28: case 29:
	o_ptr->art_flags3 |= TR3_SHOW_MODS;
	o_ptr->to_h += 4 + (randint(11));
	o_ptr->to_d += 4 + (randint(11));
	break;
    case 30:
        o_ptr->art_flags3 |= TR3_NO_MAGIC;
        break;
    case 31:
        o_ptr->art_flags3 |= TR3_NO_TELE;
        break;
    }


}


void random_slay (object_type * o_ptr, bool is_scroll)
{
	if (artifact_bias == BIAS_CHAOS && !(o_ptr->tval == TV_BOW))
	{
		if (!(o_ptr->art_flags1 & TR1_CHAOTIC))
		{
			o_ptr->art_flags1 |= TR1_CHAOTIC;
			if (randint(2)==1) return;
		}
        }

	else if (artifact_bias == BIAS_PRIESTLY &&
           (o_ptr->tval == TV_SWORD || o_ptr->tval == TV_POLEARM || o_ptr->tval == TV_AXE) &&
	  !(o_ptr->art_flags3 & TR3_BLESSED))
	{
		/* A free power for "priestly" random artifacts */
		o_ptr->art_flags3 |= TR3_BLESSED;
	}

	else if (artifact_bias == BIAS_NECROMANTIC && !(o_ptr->tval == TV_BOW))
	{
		if (!(o_ptr->art_flags1 & TR1_VAMPIRIC))
		{
			o_ptr->art_flags1 |= TR1_VAMPIRIC;
			if (randint(2)==1) return;
		}
		if (!(o_ptr->art_flags1 & TR1_BRAND_POIS) && (randint(2)==1))
		{
			o_ptr->art_flags1 |= TR1_BRAND_POIS;
			if (randint(2)==1) return;
		}
	}

	else if (artifact_bias == BIAS_RANGER && !(o_ptr->tval == TV_BOW))
	{
		if (!(o_ptr->art_flags1 & TR1_SLAY_ANIMAL))
		{
			o_ptr->art_flags1 |= TR1_SLAY_ANIMAL;
			if (randint(2)==1) return;
		}
	}

	else if (artifact_bias == BIAS_ROGUE && !(o_ptr->tval == TV_BOW))
	{
		if (!(o_ptr->art_flags1 & TR1_BRAND_POIS))
		{
			o_ptr->art_flags1 |= TR1_BRAND_POIS;
			if (randint(2)==1) return;
		}
	}

	else if (artifact_bias == BIAS_POIS && !(o_ptr->tval == TV_BOW))
	{
		if (!(o_ptr->art_flags1 & TR1_BRAND_POIS))
		{
			o_ptr->art_flags1 |= TR1_BRAND_POIS;
			if (randint(2)==1) return;
		}
	}

	else if (artifact_bias == BIAS_FIRE && !(o_ptr->tval == TV_BOW))
	{
		if (!(o_ptr->art_flags1 & TR1_BRAND_FIRE))
		{
			o_ptr->art_flags1 |= TR1_BRAND_FIRE;
			if (randint(2)==1) return;
		}
	}

	else if (artifact_bias == BIAS_COLD && !(o_ptr->tval == TV_BOW))
	{
		if (!(o_ptr->art_flags1 & TR1_BRAND_COLD))
		{
			o_ptr->art_flags1 |= TR1_BRAND_COLD;
			if (randint(2)==1) return;
		}
	}

	else if (artifact_bias == BIAS_ELEC && !(o_ptr->tval == TV_BOW))
	{
		if (!(o_ptr->art_flags1 & TR1_BRAND_ELEC))
		{
			o_ptr->art_flags1 |= TR1_BRAND_ELEC;
			if (randint(2)==1) return;
		}
	}

	else if (artifact_bias == BIAS_ACID && !(o_ptr->tval == TV_BOW))
	{
		if (!(o_ptr->art_flags1 & TR1_BRAND_ACID))
		{
			o_ptr->art_flags1 |= TR1_BRAND_ACID;
			if (randint(2)==1) return;
		}
	}

	else if (artifact_bias == BIAS_LAW && !(o_ptr->tval == TV_BOW))
	{
		if (!(o_ptr->art_flags1 & TR1_SLAY_EVIL))
		{
			o_ptr->art_flags1 |= TR1_SLAY_EVIL;
			if (randint(2)==1) return;
		}
		if (!(o_ptr->art_flags1 & TR1_SLAY_UNDEAD))
		{
			o_ptr->art_flags1 |= TR1_SLAY_UNDEAD;
			if (randint(2)==1) return;
		}
		if (!(o_ptr->art_flags1 & TR1_SLAY_DEMON))
		{
			o_ptr->art_flags1 |= TR1_SLAY_DEMON;
			if (randint(2)==1) return;
		}
	}

  if (!(o_ptr->tval == TV_BOW))
  {
    switch (randint(34))
    {
    case 1: case 2:
	o_ptr->art_flags1 |= TR1_SLAY_ANIMAL;
/*  if (is_scroll) msg_print("You start hating animals.");*/
	break;
    case 3: case 4:
	o_ptr->art_flags1 |= TR1_SLAY_EVIL;
/*  if (is_scroll) msg_print("You hate evil creatures.");*/
	if (!artifact_bias && (randint(2)==1))
	    artifact_bias = BIAS_LAW;
	else if (!artifact_bias && (randint(9)==1))
	    artifact_bias = BIAS_PRIESTLY;
	break;
    case 5: case 6:
	o_ptr->art_flags1 |= TR1_SLAY_UNDEAD;
/*  if (is_scroll) msg_print("You hate undead creatures.");*/
	if (!artifact_bias && (randint(9)==1))
	    artifact_bias = BIAS_PRIESTLY;
	break;
    case 7: case 8:
	o_ptr->art_flags1 |= TR1_SLAY_DEMON;
/*  if (is_scroll) msg_print("You hate demons.");*/
	if (!artifact_bias && (randint(9)==1))
	    artifact_bias = BIAS_PRIESTLY;
	break;
    case 9: case 10:
	o_ptr->art_flags1 |= TR1_SLAY_ORC;
/*  if (is_scroll) msg_print("You hate orcs.");*/
	break;
    case 11: case 12:
	o_ptr->art_flags1 |= TR1_SLAY_TROLL;
/*  if (is_scroll) msg_print("You hate trolls.");*/
	break;
    case 13: case 14:
	o_ptr->art_flags1 |= TR1_SLAY_GIANT;
/*  if (is_scroll) msg_print("You hate giants.");*/
	break;
    case 15: case 16:
	o_ptr->art_flags1 |= TR1_SLAY_DRAGON;
/*  if (is_scroll) msg_print("You hate dragons.");*/
	break;
    case 17: 
	o_ptr->art_flags1 |= TR1_KILL_DRAGON;
/*  if (is_scroll) msg_print("You feel an intense hatred of dragons.");*/
	break;
    case 18:  case 19:
	if (o_ptr->tval == TV_SWORD)
	    {   o_ptr->art_flags1 |= TR1_VORPAL;
/*      if (is_scroll) msg_print("It looks extremely sharp!");*/
		if (!artifact_bias && (randint(9)==1))
		    artifact_bias = BIAS_WARRIOR;
	    }
	else random_slay(o_ptr, is_scroll);
	break;
    case 20:
	o_ptr->art_flags1 |= TR1_IMPACT;
/*  if (is_scroll) msg_print("The ground trembles beneath you.");*/
	break;
    case 21: case 22:
	o_ptr->art_flags1 |= TR1_BRAND_FIRE;
/*  if (is_scroll) msg_print("It feels hot!");*/
	if (!artifact_bias)
	    artifact_bias = BIAS_FIRE;
	break;
    case 23: case 24:
	o_ptr->art_flags1 |= TR1_BRAND_COLD;
/*  if (is_scroll) msg_print("It feels cold!");*/
	if (!artifact_bias)
	    artifact_bias = BIAS_COLD;
	break;
    case 25: case 26:
	o_ptr->art_flags1 |= TR1_BRAND_ELEC;
/*  if (is_scroll) msg_print("Ouch! You get zapped!");*/
	if (!artifact_bias)
	    artifact_bias = BIAS_ELEC;
	break;
    case 27: case 28:
	o_ptr->art_flags1 |= TR1_BRAND_ACID;
/*  if (is_scroll) msg_print("Its smell makes you feel dizzy.");*/
	if (!artifact_bias)
	    artifact_bias = BIAS_ACID;
	break;
    case 29: case 30:
	o_ptr->art_flags1 |= TR1_BRAND_POIS;
/*  if (is_scroll) msg_print("It smells rotten.");*/
	if (!artifact_bias && (randint(3)!=1))
	    artifact_bias = BIAS_POIS;
	else if (!artifact_bias && randint(6)==1)
	    artifact_bias = BIAS_NECROMANTIC;
	else if (!artifact_bias)
	    artifact_bias = BIAS_ROGUE;
	break;
    case 31: case 32:
	o_ptr->art_flags1 |= TR1_VAMPIRIC;
/*  if (is_scroll) msg_print("You think it bit you!");*/
	if (!artifact_bias)
	    artifact_bias = BIAS_NECROMANTIC;
	break;
	default:
	o_ptr->art_flags1 |= TR1_CHAOTIC;
/*  if (is_scroll) msg_print("It looks very confusing.");*/
	if (!artifact_bias)
	    artifact_bias = BIAS_CHAOS;
	break;
      }
  }
  else
  {
    switch (randint(6))
    {
	case 1: case 2: case 3:
	o_ptr->art_flags3 |= TR3_XTRA_MIGHT;
/*  if (is_scroll) msg_print("It looks mightier than before."); */
	if (!artifact_bias && randint(9)==1)
	    artifact_bias = BIAS_RANGER;
	break;
	default:
	o_ptr->art_flags3 |= TR3_XTRA_SHOTS;
/*  if (is_scroll) msg_print("It seems faster!"); */
	if (!artifact_bias && randint(9)==1)
	    artifact_bias = BIAS_RANGER;
	break;
    }
  }
}


/*
 * Identify an object in the inventory (or on the floor)
 * This routine does *not* automatically combine objects.
 * Returns TRUE if something was identified, else FALSE.
 */
bool ident_spell(void)
{
	int             item;

	object_type     *o_ptr;

	char            o_name[80];

	cptr q, s;

	/* Get an item */
	q = "Identify which item? ";
	s = "You have nothing to identify.";
	if (!get_item(&item, q, s, (USE_EQUIP | USE_INVEN | USE_FLOOR))) return (FALSE);

	/* Get the item (in the pack) */
	if (item >= 0)
	{
		o_ptr = &inventory[item];
	}

	/* Get the item (on the floor) */
	else
	{
		o_ptr = &o_list[0 - item];
	}


	/* Identify it fully */
	object_aware(o_ptr);
	object_known(o_ptr);

	/* Recalculate bonuses */
	p_ptr->update |= (PU_BONUS);

	/* Combine / Reorder the pack (later) */
	p_ptr->notice |= (PN_COMBINE | PN_REORDER);

	/* Window stuff */
	p_ptr->window |= (PW_INVEN | PW_EQUIP | PW_PLAYER);

	/* Description */
	object_desc(o_name, o_ptr, TRUE, 3);

	/* Describe */
	if (item >= INVEN_WIELD)
	{
		msg_format("%^s: %s (%c).",
			   describe_use(item), o_name, index_to_label(item));
	}
	else if (item >= 0)
	{
		msg_format("In your pack: %s (%c).",
			   o_name, index_to_label(item));
	}
	else
	{
		msg_format("On the ground: %s.",
			   o_name);
	}

        /* If the item was an artifact, and if the auto-note is selected, write a message. */
        if (take_notes && auto_notes && (artifact_p(o_ptr) || o_ptr->name1))
        {
		char note[80];
		char item_name[80];
		object_desc(item_name, o_ptr, FALSE, 0);
  	  
		/* Build note and write */
		sprintf(note, "Found The %s", item_name);
	   
		add_note(note, 'A');	
        }
        /* Process the appropriate hooks */
        process_hooks(HOOK_IDENTIFY, item);

	/* Something happened */
	return (TRUE);
}



/*
 * Fully "identify" an object in the inventory  -BEN-
 * This routine returns TRUE if an item was identified.
 */
bool identify_fully(void)
{
	int             item;
	object_type     *o_ptr;
	char            o_name[80];

	cptr q, s;

	/* Get an item */
	q = "Identify which item? ";
	s = "You have nothing to identify.";
	if (!get_item(&item, q, s, (USE_EQUIP | USE_INVEN | USE_FLOOR))) return (FALSE);

	/* Get the item (in the pack) */
	if (item >= 0)
	{
		o_ptr = &inventory[item];
	}

	/* Get the item (on the floor) */
	else
	{
		o_ptr = &o_list[0 - item];
	}


	/* Identify it fully */
	object_aware(o_ptr);
	object_known(o_ptr);
		
	/* Mark the item as fully known */
	o_ptr->ident |= (IDENT_MENTAL);

	/* Recalculate bonuses */
	p_ptr->update |= (PU_BONUS);

	/* Combine / Reorder the pack (later) */
	p_ptr->notice |= (PN_COMBINE | PN_REORDER);

	/* Window stuff */
	p_ptr->window |= (PW_INVEN | PW_EQUIP | PW_PLAYER);

	/* Handle stuff */
	handle_stuff();

	/* Description */
	object_desc(o_name, o_ptr, TRUE, 3);

	/* Describe */
	if (item >= INVEN_WIELD)
	{
		msg_format("%^s: %s (%c).",
			   describe_use(item), o_name, index_to_label(item));
	}
	else if (item >= 0)
	{
		msg_format("In your pack: %s (%c).",
			   o_name, index_to_label(item));
	}
	else
	{
		msg_format("On the ground: %s.",
			   o_name);
	}
	
	/* Artifacts make us happy */
	if(artifact_p(o_ptr))
	  {
	  artifact_type *k_ptr;
	  k_ptr = &a_info[o_ptr->k_idx];
	  
	  /* Rare, expensive artifacts make us very happy */
	  p_ptr->morale += k_ptr->cost * k_ptr->rarity / (k_ptr->level * 1000);
	  }
        /* If the item was an artifact, and if the auto-note is selected, write a message. */
        if (take_notes && auto_notes && (artifact_p(o_ptr) || o_ptr->name1))
        {
		char note[80];
		char item_name[80];
		object_desc(item_name, o_ptr, FALSE, 0);
  	  
		/* Build note and write */
		sprintf(note, "Found The %s", item_name);
	   
		add_note(note, 'A');	
        }
        /* Process the appropriate hooks */
        process_hooks(HOOK_IDENTIFY, item);

	/* Describe it fully */
        identify_fully_aux(o_ptr, NULL);

	/* Success */
	return (TRUE);
}




/*
 * Hook for "get_item()".  Determine if something is rechargable.
 */
bool item_tester_hook_recharge(object_type *o_ptr)
{
        u32b f1, f2, f3, f4, f5, esp;

        /* Extract the flags */
        object_flags(o_ptr, &f1, &f2, &f3, &f4, &f5, &esp);

        /* Some objects cannot be recharged */
        if (f4 & TR4_NO_RECHARGE) return (FALSE);

        /* Recharge staffs */
        if (o_ptr->tval == TV_STAFF) return (TRUE);

	/* Recharge wands */
	if (o_ptr->tval == TV_WAND) return (TRUE);

	/* Hack -- Recharge rods */
        if (o_ptr->tval == TV_ROD_MAIN) return (TRUE);

	/* Nope */
	return (FALSE);
}


/*
 * Recharge a wand/staff/rod from the pack or on the floor.
 * This function has been rewritten in Oangband. -LM-
 *
 * Mage -- Recharge I --> recharge(90)
 * Mage -- Recharge II --> recharge(150)
 * Mage -- Recharge III --> recharge(220)
 *
 * Priest or Necromancer -- Recharge --> recharge(140)
 *
 * Scroll of recharging --> recharge(130)
 * Scroll of *recharging* --> recharge(200)
 *
 * It is harder to recharge high level, and highly charged wands, 
 * staffs, and rods.  The more wands in a stack, the more easily and 
 * strongly they recharge.  Staffs, however, each get fewer charges if 
 * stacked.
 *
 * XXX XXX XXX Beware of "sliding index errors".
 */
bool recharge(int power)
{
	int recharge_strength, recharge_amount;
        int item, lev;

	bool fail = FALSE;
	byte fail_type = 1;


	cptr q, s;

        u32b f1, f2, f3, f4, f5, esp;
	char o_name[80];

	object_type *o_ptr;
	object_kind *k_ptr;

	/* Only accept legal items */
	item_tester_hook = item_tester_hook_recharge;

	/* Get an item */
	q = "Recharge which item? ";
	s = "You have nothing to recharge.";
	if (!get_item(&item, q, s, (USE_INVEN | USE_FLOOR))) return (FALSE);

	/* Get the item (in the pack) */
	if (item >= 0)
	{
		o_ptr = &inventory[item];
	}

	/* Get the item (on the floor) */
	else
	{
		o_ptr = &o_list[0 - item];
	}

        /* Extract the flags */
        object_flags(o_ptr, &f1, &f2, &f3, &f4, &f5, &esp);

	/* Extract the object "level" */
	lev = k_info[o_ptr->k_idx].level;
        k_ptr = &k_info[o_ptr->k_idx];

	/* Recharge a rod */
        if (o_ptr->tval == TV_ROD_MAIN)
	{
		/* Extract a recharge strength by comparing object level to power. */
		recharge_strength = ((power > lev) ? (power - lev) : 0) / 5;
                
                /* Paranoia */
                if (recharge_strength < 0) recharge_strength = 0;

		/* Back-fire */
                if ((rand_int(recharge_strength) == 0)&&(!(f4 & TR4_RECHARGE)))
		{
			/* Activate the failure code. */
			fail = TRUE;
		}

		/* Recharge */
		else
		{
			/* Recharge amount */
			recharge_amount = (power * damroll(3, 2));

			/* Recharge by that amount */
                        if (o_ptr->timeout + recharge_amount < o_ptr->pval2)
                                o_ptr->timeout += recharge_amount;
			else
                                o_ptr->timeout = o_ptr->pval2;
		}
	}


	/* Recharge wand/staff */
	else
	{
		/* Extract a recharge strength by comparing object level to power. 
		 * Divide up a stack of wands' charges to calculate charge penalty.
		 */
		if ((o_ptr->tval == TV_WAND) && (o_ptr->number > 1))
			recharge_strength = (100 + power - lev - 
			(8 * o_ptr->pval / o_ptr->number)) / 15;

		/* All staffs, unstacked wands. */
		else recharge_strength = (100 + power - lev - 
			(8 * o_ptr->pval)) / 15;


		/* Back-fire XXX XXX XXX */
                if (((rand_int(recharge_strength) == 0) && (!(f4 & TR4_RECHARGE))) ||
                    (f4 & TR4_NO_RECHARGE))
		{
			/* Activate the failure code. */
			fail = TRUE;
		}

		/* If the spell didn't backfire, recharge the wand or staff. */
		else
		{
			/* Recharge based on the standard number of charges. */
                        recharge_amount = randint((power / (lev + 2)) + 1);

			/* Multiple wands in a stack increase recharging somewhat. */
			if ((o_ptr->tval == TV_WAND) && (o_ptr->number > 1))
			{
				recharge_amount += 
					(randint(recharge_amount * (o_ptr->number - 1))) / 2;
				if (recharge_amount < 1) recharge_amount = 1;
				if (recharge_amount > 12) recharge_amount = 12;
			}

			/* But each staff in a stack gets fewer additional charges, 
			 * although always at least one.
			 */
			if ((o_ptr->tval == TV_STAFF) && (o_ptr->number > 1))
			{
				recharge_amount /= o_ptr->number;
				if (recharge_amount < 1) recharge_amount = 1;
			}

			/* Recharge the wand or staff. */
			o_ptr->pval += recharge_amount;

                        if(!(f4 & TR4_RECHARGE))
                        {
                                /* Hack -- we no longer "know" the item */
                                o_ptr->ident &= ~(IDENT_KNOWN);
                        }

			/* Hack -- we no longer think the item is empty */
			o_ptr->ident &= ~(IDENT_EMPTY);
		}
	}

        /* Mark as recharged -- For alchemists */
        o_ptr->art_flags4 |= TR4_RECHARGED;

	/* Inflict the penalties for failing a recharge. */
	if (fail)
	{
		/* Artifacts are never destroyed. */
		if (artifact_p(o_ptr))
		{
			object_desc(o_name, o_ptr, TRUE, 0);
			msg_format("The recharging backfires - %s is completely drained!", o_name);

			/* Artifact rods. */
                        if (o_ptr->tval == TV_ROD_MAIN) 
                                o_ptr->timeout = 0;

			/* Artifact wands and staffs. */
			else if ((o_ptr->tval == TV_WAND) || (o_ptr->tval == TV_STAFF)) 
				o_ptr->pval = 0;
		}
		else 
		{
			/* Get the object description */
			object_desc(o_name, o_ptr, FALSE, 0);

			/*** Determine Seriousness of Failure ***/

			/* Mages recharge objects more safely. */
			if (p_ptr->pclass == CLASS_MAGE)
			{
				/* 10% chance to blow up one rod, otherwise draining. */
                                if (o_ptr->tval == TV_ROD_MAIN)
				{
					if (randint(10) == 1) fail_type = 2;
					else fail_type = 1;
				}
				/* 75% chance to blow up one wand, otherwise draining. */
				else if (o_ptr->tval == TV_WAND)
				{
					if (randint(3) != 1) fail_type = 2;
					else fail_type = 1;
				}
				/* 50% chance to blow up one staff, otherwise no effect. */
				else if (o_ptr->tval == TV_STAFF)
				{
					if (randint(2) == 1) fail_type = 2;
					else fail_type = 0;
				}
			}

			/* All other classes get no special favors. */
			else
			{
				/* 33% chance to blow up one rod, otherwise draining. */
                                if (o_ptr->tval == TV_ROD_MAIN)
				{
					if (randint(3) == 1) fail_type = 2;
					else fail_type = 1;
				}
				/* 20% chance of the entire stack, else destroy one wand. */
				else if (o_ptr->tval == TV_WAND)
				{
					if (randint(5) == 1) fail_type = 3;
					else fail_type = 2;
				}
				/* Blow up one staff. */
				else if (o_ptr->tval == TV_STAFF)
				{
					fail_type = 2;
				}
			}

			/*** Apply draining and destruction. ***/

			/* Drain object or stack of objects. */
			if (fail_type == 1)
			{
                                if (o_ptr->tval == TV_ROD_MAIN)
				{
					msg_print("The recharge backfires, draining the rod further!");
					if (o_ptr->timeout < 10000) 
                                                o_ptr->timeout = 0;
				}
				else if (o_ptr->tval == TV_WAND)
				{
					msg_format("You save your %s from destruction, but all charges are lost.", o_name);
					o_ptr->pval = 0;
				}
				/* Staffs aren't drained. */
			}

			/* Destroy an object or one in a stack of objects. */
			if (fail_type == 2)
			{
				if (o_ptr->number > 1)
					msg_format("Wild magic consumes one of your %s!", o_name);
				else
					msg_format("Wild magic consumes your %s!", o_name);

				/* Reduce rod stack maximum timeout, drain wands. */
				if (o_ptr->tval == TV_WAND) o_ptr->pval = 0;

				/* Reduce and describe inventory */
				if (item >= 0)
				{
					inven_item_increase(item, -1);
					inven_item_describe(item);
					inven_item_optimize(item);
				}

				/* Reduce and describe floor item */
				else
				{
					floor_item_increase(0 - item, -1);
					floor_item_describe(0 - item);
					floor_item_optimize(0 - item);
				}
			}

			/* Destroy all memebers of a stack of objects. */
			if (fail_type == 3)
			{
				if (o_ptr->number > 1)
					msg_format("Wild magic consumes all your %s!", o_name);
				else
					msg_format("Wild magic consumes your %s!", o_name);


				/* Reduce and describe inventory */
				if (item >= 0)
				{
					inven_item_increase(item, -999);
					inven_item_describe(item);
					inven_item_optimize(item);
				}

				/* Reduce and describe floor item */
				else
				{
					floor_item_increase(0 - item, -999);
					floor_item_describe(0 - item);
					floor_item_optimize(0 - item);
				}
			}
		}
	}


	/* Combine / Reorder the pack (later) */
	p_ptr->notice |= (PN_COMBINE | PN_REORDER);

	/* Window stuff */
	p_ptr->window |= (PW_INVEN);

	/* Something was done */
	return (TRUE);
}



/*
 * Apply a "project()" directly to all viewable monsters
 *
 * Note that affected monsters are NOT auto-tracked by this usage.
 */
bool project_hack(int typ, int dam)
{
	int     i, x, y;
	int     flg = PROJECT_JUMP | PROJECT_KILL | PROJECT_HIDE;
	bool    obvious = FALSE;


	/* Affect all (nearby) monsters */
	for (i = 1; i < m_max; i++)
	{
		monster_type *m_ptr = &m_list[i];

		/* Paranoia -- Skip dead monsters */
		if (!m_ptr->r_idx) continue;

		/* Location */
		y = m_ptr->fy;
		x = m_ptr->fx;

		/* Require line of sight */
		if (!player_has_los_bold(y, x)) continue;

		/* Jump directly to the target monster */
		if (project(0, 0, y, x, dam, typ, flg)) obvious = TRUE;
	}

	/* Result */
	return (obvious);
}

/*
 * Apply a "project()" a la meteor shower
 */
void project_meteor(int radius, int typ, int dam, u32b flg)
{
        int x, y, dx, dy, d, count = 0, i;
        int b = radius + randint(radius); 
        for (i = 0; i < b; i++) {
		for (count = 0; count < 1000; count++)
		{
                        x = px - 5 + randint(10);
                        y = py - 5 + randint(10);
			if ((x < 0) || (x >= cur_wid) ||
			    (y < 0) || (y >= cur_hgt)) continue;
                        dx = (px > x) ? (px - x) : (x - px);
                        dy = (py > y) ? (py - y) : (y - py);
                        /* Approximate distance */
                        d = (dy > dx) ? (dy + (dx>>1)) : (dx + (dy>>1));
			if ((d <= 5) && (player_has_los_bold(y, x))) break;
		}
                if (count >= 1000)   break;
                project(0, 2, y, x, dam, typ, PROJECT_JUMP | flg);
        }
}


/*
 * Speed monsters
 */
bool speed_monsters(void)
{
	return (project_hack(GF_OLD_SPEED, p_ptr->lev));
}

/*
 * Slow monsters
 */
bool slow_monsters(void)
{
	return (project_hack(GF_OLD_SLOW, p_ptr->lev));
}

/*
 * Paralyzation monsters
 */
bool conf_monsters(void)
{
        return (project_hack(GF_OLD_CONF, p_ptr->lev));
}

/*
 * Sleep monsters
 */
bool sleep_monsters(void)
{
	return (project_hack(GF_OLD_SLEEP, p_ptr->lev));
}

/*
 * Scare monsters
 */
bool scare_monsters(void)
{
        return (project_hack(GF_FEAR, p_ptr->lev));
}


/*
 * Banish evil monsters
 */
bool banish_evil(int dist)
{
	return (project_hack(GF_AWAY_EVIL, dist));
}


/*
 * Turn undead
 */
bool turn_undead(void)
{
	return (project_hack(GF_TURN_UNDEAD, p_ptr->lev));
}


/*
 * Dispel undead monsters
 */
bool dispel_undead(int dam)
{
	return (project_hack(GF_DISP_UNDEAD, dam));
}

/*
 * Dispel evil monsters
 */
bool dispel_evil(int dam)
{
	return (project_hack(GF_DISP_EVIL, dam));
}

/*
 * Dispel good monsters
 */
bool dispel_good(int dam)
{
    return (project_hack(GF_DISP_GOOD, dam));
}

/*
 * Dispel all monsters
 */
bool dispel_monsters(int dam)
{
	return (project_hack(GF_DISP_ALL, dam));
}

/*
 * Dispel 'living' monsters
 */
bool dispel_living(int dam)
{
	return (project_hack(GF_DISP_LIVING, dam));
}

/*
 * Dispel demons
 */
bool dispel_demons(int dam)
{
	return (project_hack(GF_DISP_DEMON, dam));
}


/*
 * Wake up all monsters, and speed up "los" monsters.
 */
void aggravate_monsters(int who)
{
	int     i;
	bool    sleep = FALSE;
	bool    speed = FALSE;


	/* Aggravate everyone nearby */
	for (i = 1; i < m_max; i++)
	{
		monster_type    *m_ptr = &m_list[i];
                monster_race    *r_ptr = race_inf(m_ptr);

		/* Paranoia -- Skip dead monsters */
		if (!m_ptr->r_idx) continue;

		/* Skip aggravating monster (or player) */
		if (i == who) continue;

		/* Wake up nearby sleeping monsters */
		if (m_ptr->cdis < MAX_SIGHT * 2)
		{
			/* Wake up */
			if (m_ptr->csleep)
			{
				/* Wake up */
				m_ptr->csleep = 0;
				sleep = TRUE;
			}
		}

		/* Speed up monsters in line of sight */
		if (player_has_los_bold(m_ptr->fy, m_ptr->fx))
		{
			/* Speed up (instantly) to racial base + 10 */
			if (m_ptr->mspeed < r_ptr->speed + 10)
			{
				/* Speed up */
				m_ptr->mspeed = r_ptr->speed + 10;
				speed = TRUE;
			}

			/* Pets may get angry (50% chance) */
                        if (is_friend(m_ptr))
			{
				if (randint(2)==1)
				{
                                        change_side(m_ptr);
				}
			}
		}
	}

	/* Messages */
	if (speed) msg_print("You feel a sudden stirring nearby!");
	else if (sleep) msg_print("You hear a sudden stirring in the distance!");
}

/*
 * Inflict dam damage of type typee to all monster of the given race
 */
bool invoke(int dam, int typee)
{
	int     i;
	char    typ;
	bool    result = FALSE;
	int     msec = delay_factor * delay_factor * delay_factor;

        if (dungeon_flags1 & LF1_NO_GENO) return(FALSE);

        /* Hack -- when you are fated to die, you cant cheat :) */
        if (dungeon_type == DUNGEON_DEATH)
        {
                msg_print("A mysterious force stops the genocide.");
                return FALSE;
        }

	/* Mega-Hack -- Get a monster symbol */
	(void)(get_com("Choose a monster race (by symbol) to genocide: ", &typ));

	/* Delete the monsters of that "type" */
	for (i = 1; i < m_max; i++)
	{
		monster_type    *m_ptr = &m_list[i];
                monster_race    *r_ptr = race_inf(m_ptr);

		/* Paranoia -- Skip dead monsters */
		if (!m_ptr->r_idx) continue;

		/* Hack -- Skip Unique Monsters */
		if (r_ptr->flags1 & (RF1_UNIQUE)) continue;

		/* Hack -- Skip Quest Monsters */
		if (r_ptr->flags1 & RF1_QUESTOR) continue;

		/* Skip "wrong" monsters */
		if (r_ptr->d_char != typ) continue;

                project_m(0, 0, m_ptr->fy, m_ptr->fx, dam, typee);

		/* Visual feedback */
		move_cursor_relative(py, px);

		/* Window stuff */
		p_ptr->window |= (PW_PLAYER);

		/* Handle */
		handle_stuff();

		/* Fresh */
		Term_fresh();

		/* Delay */
		Term_xtra(TERM_XTRA_DELAY, msec);

		/* Take note */
		result = TRUE;
	}

	return (result);
}


/*
 * Delete all non-unique/non-quest monsters of a given "type" from the level
 */
bool genocide_aux(bool player_cast, char typ)
{
	int     i;
	bool    result = FALSE;
	int     msec = delay_factor * delay_factor * delay_factor;

	/* Delete the monsters of that "type" */
	for (i = 1; i < m_max; i++)
	{
		monster_type    *m_ptr = &m_list[i];
                monster_race    *r_ptr = race_inf(m_ptr);

		/* Paranoia -- Skip dead monsters */
		if (!m_ptr->r_idx) continue;

		/* Hack -- Skip Unique Monsters */
		if (r_ptr->flags1 & (RF1_UNIQUE)) continue;

		/* Hack -- Skip Quest Monsters */
		if (r_ptr->flags1 & RF1_QUESTOR) continue;

		/* Skip "wrong" monsters */
		if (r_ptr->d_char != typ) continue;

                /* Oups */
                if (r_ptr->flags2 & RF2_DEATH_ORB)
                {
                        int wx, wy;
                        int attempts = 500;

                        monster_race_desc(r_name, m_ptr->r_idx, 0);

                        do
                        {
                                scatter(&wy, &wx, m_ptr->fy, m_ptr->fx, 10, 0);
                        }
                        while (!(in_bounds(wy,wx) && cave_floor_bold(wy,wx)) && --attempts);

                        if (place_monster_aux(wy, wx, m_ptr->r_idx, FALSE, TRUE, MSTATUS_ENEMY))
                        {
                                cmsg_format(TERM_L_BLUE, "The spell seems to produce an ... interreting effect on the %s.", r_name);
                        }

                        return TRUE;
                }

		/* Delete the monster */
		delete_monster_idx(i);

		if (player_cast)
		{
			/* Take damage */
			take_hit(randint(4), "the strain of casting Genocide");
		}

		/* Visual feedback */
		move_cursor_relative(py, px);

		/* Redraw */
		p_ptr->redraw |= (PR_HP);

		/* Window stuff */
		p_ptr->window |= (PW_PLAYER);

		/* Handle */
		handle_stuff();

		/* Fresh */
		Term_fresh();

		/* Delay */
		Term_xtra(TERM_XTRA_DELAY, msec);

		/* Take note */
		result = TRUE;
	}

        return (result);
}

bool genocide(bool player_cast)
{
	char    typ;

        if (dungeon_flags1 & LF1_NO_GENO) return(FALSE);

        /* Hack -- when you are fated to die, you cant cheat :) */
        if (dungeon_type == DUNGEON_DEATH)
        {
                msg_print("A mysterious force stops the genocide.");
                return FALSE;
        }

	/* Mega-Hack -- Get a monster symbol */
	(void)(get_com("Choose a monster race (by symbol) to genocide: ", &typ));

        return (genocide_aux(player_cast, typ));
}


/*
 * Delete all nearby (non-unique) monsters
 */
bool mass_genocide(bool player_cast)
{
	int     i;

	bool    result = FALSE;

	int     msec = delay_factor * delay_factor * delay_factor;

        if (dungeon_flags1 & LF1_NO_GENO) return(FALSE);

        /* Hack -- when you are fated to die, you cant cheat :) */
        if (dungeon_type == DUNGEON_DEATH)
        {
                msg_print("A mysterious force stops the genocide.");
                return FALSE;
        }

	/* Delete the (nearby) monsters */
	for (i = 1; i < m_max; i++)
	{
		monster_type    *m_ptr = &m_list[i];
                monster_race    *r_ptr = race_inf(m_ptr);

		/* Paranoia -- Skip dead monsters */
		if (!m_ptr->r_idx) continue;

		/* Hack -- Skip unique monsters */
		if (r_ptr->flags1 & (RF1_UNIQUE)) continue;

		/* Hack -- Skip Quest Monsters */
		if (r_ptr->flags1 & RF1_QUESTOR) continue;

		/* Skip distant monsters */
		if (m_ptr->cdis > MAX_SIGHT) continue;

                /* Oups */
                if (r_ptr->flags2 & RF2_DEATH_ORB)
                {
                        int wx, wy;
                        int attempts = 500;

                        monster_race_desc(r_name, m_ptr->r_idx, 0);

                        do
                        {
                                scatter(&wy, &wx, m_ptr->fy, m_ptr->fx, 10, 0);
                        }
                        while (!(in_bounds(wy,wx) && cave_floor_bold(wy,wx)) && --attempts);

                        if (place_monster_aux(wy, wx, m_ptr->r_idx, FALSE, TRUE, MSTATUS_ENEMY))
                        {
                                cmsg_format(TERM_L_BLUE, "The spell seems to produce an ... interreting effect on the %s.", r_name);
                        }

                        return TRUE;
                }

		/* Delete the monster */
		delete_monster_idx(i);

		if (player_cast)
		{
			/* Hack -- visual feedback */
			take_hit(randint(3), "the strain of casting Mass Genocide");
		}

		move_cursor_relative(py, px);

		/* Redraw */
		p_ptr->redraw |= (PR_HP);

		/* Window stuff */
		p_ptr->window |= (PW_PLAYER);

		/* Handle */
		handle_stuff();

		/* Fresh */
		Term_fresh();

		/* Delay */
		Term_xtra(TERM_XTRA_DELAY, msec);

		/* Note effect */
		result = TRUE;
	}

	return (result);
}

/* Probe a monster */
void do_probe(int m_idx)
{
        char m_name[80];
        monster_type *m_ptr = &m_list[m_idx];

        /* Get "the monster" or "something" */
        monster_desc(m_name, m_ptr, 0x04);

        /* Describe the monster */
        if (!wizard && (m_ptr->status != MSTATUS_COMPANION)) msg_format("%^s has %d hit points.", m_name, m_ptr->hp);
        else
        {
                int i;
                char t_name[80];
                msg_format("%^s has %d(%d) hit points, %d ac, %d speed.", m_name, m_ptr->hp, m_ptr->maxhp, m_ptr->ac, m_ptr->mspeed - 110);
                msg_format("%^s attacks with:", m_name);

                for (i = 0; i < 4; i++)
                {
                        msg_format("    Blow %d: %dd%d", i, m_ptr->blow[i].d_dice, m_ptr->blow[i].d_side);
                }

                if (m_ptr->target > 0)
                        monster_desc(t_name, &m_list[m_ptr->target], 0x04);
                else if (!m_ptr->target)
                        sprintf(t_name, "you");
                else
                        sprintf(t_name, "nothing");
                msg_format("%^s target is %s.", m_name, t_name);

                msg_format("%^s has %ld exp and needs %ld.", m_name, m_ptr->exp, MONSTER_EXP(m_ptr->level + 1));
        }

        /* Learn all of the non-spell, non-treasure flags */
        lore_do_probe(m_idx);
}

/*
 * Probe nearby monsters
 */
bool probing(void)
{
	int     i;

	bool    probe = FALSE;


	/* Probe all (nearby) monsters */
	for (i = 1; i < m_max; i++)
	{
		monster_type *m_ptr = &m_list[i];

		/* Paranoia -- Skip dead monsters */
		if (!m_ptr->r_idx) continue;

		/* Require line of sight */
		if (!player_has_los_bold(m_ptr->fy, m_ptr->fx)) continue;

		/* Probe visible monsters */
		if (m_ptr->ml)
		{
			/* Start the message */
			if (!probe) msg_print("Probing...");

                        /* Actualy probe */
                        do_probe(i);

			/* Probe worked */
			probe = TRUE;
		}
	}

	/* Done */
	if (probe)
	{
		msg_print("That's all.");
	}

	/* Result */
	return (probe);
}


/*
 * Wipe -- Empties a part of the dungeon
 */
void wipe(int y1, int x1, int r)
{
        int y, x, k;

	cave_type *c_ptr;

	bool flag = FALSE;

        if (dungeon_flags1 & LF1_NO_GENO)
        {
                msg_print("Not on special levels!");
                return;
        }
        if (p_ptr->inside_quest)
        {
                return;
        }

	/* Big area of affect */
	for (y = (y1 - r); y <= (y1 + r); y++)
	{
		for (x = (x1 - r); x <= (x1 + r); x++)
		{
			/* Skip illegal grids */
			if (!in_bounds(y, x)) continue;

			/* Extract the distance */
			k = distance(y1, x1, y, x);

			/* Stay in the circle of death */
			if (k > r) continue;

			/* Access the grid */
			c_ptr = &cave[y][x];

			/* Lose room and vault */
			c_ptr->info &= ~(CAVE_ROOM | CAVE_ICKY);

			/* Lose light and knowledge */
			c_ptr->info &= ~(CAVE_MARK | CAVE_GLOW);

                        if (m_list[c_ptr->m_idx].status != MSTATUS_COMPANION) delete_monster(y, x);
                        delete_object(y, x);
                        place_floor(y, x);
		}
	}


	/* Hack -- Affect player */
	if (flag)
	{
		/* Message */
		msg_print("There is a searing blast of light!");

		/* Blind the player */
		if (!p_ptr->resist_blind && !p_ptr->resist_lite)
		{
			/* Become blind */
			(void)set_blind(p_ptr->blind + 10 + randint(10));
		}
	}


	/* Mega-Hack -- Forget the view and lite */
	p_ptr->update |= (PU_UN_VIEW | PU_UN_LITE);

	/* Update stuff */
	p_ptr->update |= (PU_VIEW | PU_LITE | PU_FLOW);

	/* Update the monsters */
	p_ptr->update |= (PU_MONSTERS);

	/* Redraw map */
	p_ptr->redraw |= (PR_MAP);

	/* Window stuff */
	p_ptr->window |= (PW_OVERHEAD);
}


/*
 * The spell of destruction
 *
 * This spell "deletes" monsters (instead of "killing" them).
 *
 * Later we may use one function for both "destruction" and
 * "earthquake" by using the "full" to select "destruction".
 */
void destroy_area(int y1, int x1, int r, bool full)
{
	int y, x, k, t;

	cave_type *c_ptr;

	bool flag = FALSE;


	/* XXX XXX */
	full = full ? full : 0;

        if (dungeon_flags1 & LF1_NO_GENO)
        {
                msg_print("Not on special levels!");
                return;
        }
        if (p_ptr->inside_quest)
        {
                return;
        }

	/* Big area of affect */
	for (y = (y1 - r); y <= (y1 + r); y++)
	{
		for (x = (x1 - r); x <= (x1 + r); x++)
		{
			/* Skip illegal grids */
			if (!in_bounds(y, x)) continue;

			/* Extract the distance */
			k = distance(y1, x1, y, x);

			/* Stay in the circle of death */
			if (k > r) continue;

			/* Access the grid */
			c_ptr = &cave[y][x];

			/* Lose room and vault */
			c_ptr->info &= ~(CAVE_ROOM | CAVE_ICKY);

			/* Lose light and knowledge */
			c_ptr->info &= ~(CAVE_MARK | CAVE_GLOW);

			/* Hack -- Notice player affect */
			if ((x == px) && (y == py))
			{
				/* Hurt the player later */
				flag = TRUE;

				/* Do not hurt this grid */
				continue;
			}

			/* Hack -- Skip the epicenter */
			if ((y == y1) && (x == x1)) continue;

			/* Delete the monster (if any) */
                        if (m_list[c_ptr->m_idx].status != MSTATUS_COMPANION) delete_monster(y, x);

			/* Destroy "valid" grids */
			if (cave_valid_bold(y, x))
			{
				/* Delete objects */
				delete_object(y, x);

				/* Wall (or floor) type */
				t = rand_int(200);

				/* Granite */
				if (t < 20)
				{
					/* Create granite wall */
					c_ptr->feat = FEAT_WALL_EXTRA;
				}

				/* Quartz */
				else if (t < 70)
				{
					/* Create quartz vein */
					c_ptr->feat = FEAT_QUARTZ;
				}

				/* Magma */
				else if (t < 100)
				{
					/* Create magma vein */
					c_ptr->feat = FEAT_MAGMA;
				}

				/* Floor */
				else
				{
					/* Create floor */
					c_ptr->feat = FEAT_FLOOR;
				}
			}
		}
	}


	/* Hack -- Affect player */
	if (flag)
	{
		/* Message */
		msg_print("There is a searing blast of light!");

		/* Blind the player */
		if (!p_ptr->resist_blind && !p_ptr->resist_lite)
		{
			/* Become blind */
			(void)set_blind(p_ptr->blind + 10 + randint(10));
		}
	}


	/* Mega-Hack -- Forget the view and lite */
	p_ptr->update |= (PU_UN_VIEW | PU_UN_LITE);

	/* Update stuff */
	p_ptr->update |= (PU_VIEW | PU_LITE | PU_FLOW);

	/* Update the monsters */
	p_ptr->update |= (PU_MONSTERS);

	/* Redraw map */
	p_ptr->redraw |= (PR_MAP);

	/* Window stuff */
	p_ptr->window |= (PW_OVERHEAD);
}


/*
 * Induce an "earthquake" of the given radius at the given location.
 *
 * This will turn some walls into floors and some floors into walls.
 *
 * The player will take damage and "jump" into a safe grid if possible,
 * otherwise, he will "tunnel" through the rubble instantaneously.
 *
 * Monsters will take damage, and "jump" into a safe grid if possible,
 * otherwise they will be "buried" in the rubble, disappearing from
 * the level in the same way that they do when genocided.
 *
 * Note that thus the player and monsters (except eaters of walls and
 * passers through walls) will never occupy the same grid as a wall.
 * Note that as of now (2.7.8) no monster may occupy a "wall" grid, even
 * for a single turn, unless that monster can pass_walls or kill_walls.
 * This has allowed massive simplification of the "monster" code.
 */
void earthquake(int cy, int cx, int r)
{
	int             i, t, y, x, yy, xx, dy, dx, oy, ox;
	int             damage = 0;
	int             sn = 0, sy = 0, sx = 0;
	bool            hurt = FALSE;
	cave_type       *c_ptr;
	bool            map[32][32];

        if (p_ptr->inside_quest)
        {
                return;
        }

	/* Paranoia -- Enforce maximum range */
	if (r > 12) r = 12;

	/* Clear the "maximal blast" area */
	for (y = 0; y < 32; y++)
	{
		for (x = 0; x < 32; x++)
		{
			map[y][x] = FALSE;
		}
	}

	/* Check around the epicenter */
	for (dy = -r; dy <= r; dy++)
	{
		for (dx = -r; dx <= r; dx++)
		{
			/* Extract the location */
			yy = cy + dy;
			xx = cx + dx;

			/* Skip illegal grids */
			if (!in_bounds(yy, xx)) continue;

			/* Skip distant grids */
			if (distance(cy, cx, yy, xx) > r) continue;

			/* Access the grid */
			c_ptr = &cave[yy][xx];

			/* Lose room and vault */
			c_ptr->info &= ~(CAVE_ROOM | CAVE_ICKY);

			/* Lose light and knowledge */
			c_ptr->info &= ~(CAVE_GLOW | CAVE_MARK);

			/* Skip the epicenter */
			if (!dx && !dy) continue;

			/* Skip most grids */
			if (rand_int(100) < 85) continue;

			/* Damage this grid */
			map[16+yy-cy][16+xx-cx] = TRUE;

			/* Hack -- Take note of player damage */
			if ((yy == py) && (xx == px)) hurt = TRUE;
		}
	}

	/* First, affect the player (if necessary) */
	if (hurt)
	{
		/* Check around the player */
		for (i = 0; i < 8; i++)
		{
			/* Access the location */
			y = py + ddy[i];
			x = px + ddx[i];

			/* Skip non-empty grids */
			if (!cave_empty_bold(y, x)) continue;

			/* Important -- Skip "quake" grids */
			if (map[16+y-cy][16+x-cx]) continue;

			/* Count "safe" grids */
			sn++;

			/* Randomize choice */
			if (rand_int(sn) > 0) continue;

			/* Save the safe location */
			sy = y; sx = x;
		}

		/* Random message */
		switch (randint(3))
		{
			case 1:
			{
				msg_print("The cave ceiling collapses!");
				break;
			}
			case 2:
			{
				msg_print("The cave floor twists in an unnatural way!");
				break;
			}
			default:
			{
				msg_print("The cave quakes!  You are pummeled with debris!");
				break;
			}
		}

		/* Hurt the player a lot */
		if (!sn)
		{
			/* Message and damage */
			msg_print("You are severely crushed!");
			damage = 300;
		}

		/* Destroy the grid, and push the player to safety */
		else
		{
			/* Calculate results */
			switch (randint(3))
			{
				case 1:
				{
					msg_print("You nimbly dodge the blast!");
					damage = 0;
					break;
				}
				case 2:
				{
					msg_print("You are bashed by rubble!");
					damage = damroll(10, 4);
					(void)set_stun(p_ptr->stun + randint(50));
					break;
				}
				case 3:
				{
					msg_print("You are crushed between the floor and ceiling!");
					damage = damroll(10, 4);
					(void)set_stun(p_ptr->stun + randint(50));
					break;
				}
			}

			/* Save the old location */
			oy = py;
			ox = px;

			/* Move the player to the safe location */
			py = sy;
			px = sx;

			/* Redraw the old spot */
			lite_spot(oy, ox);

			/* Redraw the new spot */
			lite_spot(py, px);

			/* Check for new panel */
			verify_panel();
		}

		/* Important -- no wall on player */
		map[16+py-cy][16+px-cx] = FALSE;

		/* Take some damage */
		if (damage) take_hit(damage, "an earthquake");
	}


	/* Examine the quaked region */
	for (dy = -r; dy <= r; dy++)
	{
		for (dx = -r; dx <= r; dx++)
		{
			/* Extract the location */
			yy = cy + dy;
			xx = cx + dx;

			/* Skip unaffected grids */
			if (!map[16+yy-cy][16+xx-cx]) continue;

			/* Access the grid */
			c_ptr = &cave[yy][xx];

			/* Process monsters */
			if (c_ptr->m_idx)
			{
				monster_type *m_ptr = &m_list[c_ptr->m_idx];
                                monster_race *r_ptr = race_inf(m_ptr);

				/* Most monsters cannot co-exist with rock */
				if (!(r_ptr->flags2 & (RF2_KILL_WALL)) &&
				    !(r_ptr->flags2 & (RF2_PASS_WALL)))
				{
					char m_name[80];

					/* Assume not safe */
					sn = 0;

					/* Monster can move to escape the wall */
					if (!(r_ptr->flags1 & (RF1_NEVER_MOVE)))
					{
						/* Look for safety */
						for (i = 0; i < 8; i++)
						{
							/* Access the grid */
							y = yy + ddy[i];
							x = xx + ddx[i];

							/* Skip non-empty grids */
							if (!cave_empty_bold(y, x)) continue;

							/* Hack -- no safety on glyph of warding */
							if (cave[y][x].feat == FEAT_GLYPH) continue;
							if (cave[y][x].feat == FEAT_MINOR_GLYPH) continue;

							/* ... nor on the Pattern */
							if ((cave[y][x].feat <= FEAT_PATTERN_XTRA2) &&
							    (cave[y][x].feat >= FEAT_PATTERN_START))
								continue;

							/* Important -- Skip "quake" grids */
							if (map[16+y-cy][16+x-cx]) continue;

							/* Count "safe" grids */
							sn++;

							/* Randomize choice */
							if (rand_int(sn) > 0) continue;

							/* Save the safe grid */
							sy = y; sx = x;
						}
					}

					/* Describe the monster */
					monster_desc(m_name, m_ptr, 0);

					/* Scream in pain */
					msg_format("%^s wails out in pain!", m_name);

					/* Take damage from the quake */
					damage = (sn ? damroll(4, 8) : 200);

					/* Monster is certainly awake */
					m_ptr->csleep = 0;

					/* Apply damage directly */
					m_ptr->hp -= damage;

					/* Delete (not kill) "dead" monsters */
					if (m_ptr->hp < 0)
					{
						/* Message */
						msg_format("%^s is embedded in the rock!", m_name);

						/* Delete the monster */
						delete_monster(yy, xx);

						/* No longer safe */
						sn = 0;
					}

					/* Hack -- Escape from the rock */
					if (sn)
					{
						int m_idx = cave[yy][xx].m_idx;

						/* Update the new location */
						cave[sy][sx].m_idx = m_idx;

						/* Update the old location */
						cave[yy][xx].m_idx = 0;

						/* Move the monster */
						m_ptr->fy = sy;
						m_ptr->fx = sx;

						/* Update the monster (new location) */
						update_mon(m_idx, TRUE);

						/* Redraw the old grid */
						lite_spot(yy, xx);

						/* Redraw the new grid */
						lite_spot(sy, sx);
					}
				}
			}
		}
	}


	/* Examine the quaked region */
	for (dy = -r; dy <= r; dy++)
	{
		for (dx = -r; dx <= r; dx++)
		{
			/* Extract the location */
			yy = cy + dy;
			xx = cx + dx;

			/* Skip unaffected grids */
			if (!map[16+yy-cy][16+xx-cx]) continue;

			/* Access the cave grid */
			c_ptr = &cave[yy][xx];

			/* Paranoia -- never affect player */
			if ((yy == py) && (xx == px)) continue;

			/* Destroy location (if valid) */
			if (cave_valid_bold(yy, xx))
			{
				bool floor = cave_floor_bold(yy, xx);

				/* Delete objects */
				delete_object(yy, xx);

				/* Wall (or floor) type */
				t = (floor ? rand_int(100) : 200);

				/* Granite */
				if (t < 20)
				{
					/* Create granite wall */
					c_ptr->feat = FEAT_WALL_EXTRA;
				}

				/* Quartz */
				else if (t < 70)
				{
					/* Create quartz vein */
					c_ptr->feat = FEAT_QUARTZ;
				}

				/* Magma */
				else if (t < 100)
				{
					/* Create magma vein */
					c_ptr->feat = FEAT_MAGMA;
				}

				/* Floor */
				else
				{
					/* Create floor */
					c_ptr->feat = FEAT_FLOOR;
				}
			}
		}
	}


	/* Mega-Hack -- Forget the view and lite */
	p_ptr->update |= (PU_UN_VIEW | PU_UN_LITE);

	/* Update stuff */
	p_ptr->update |= (PU_VIEW | PU_LITE | PU_FLOW);

	/* Update the monsters */
	p_ptr->update |= (PU_DISTANCE);

	/* Update the health bar */
	p_ptr->redraw |= (PR_HEALTH);

	/* Redraw map */
	p_ptr->redraw |= (PR_MAP);

	/* Window stuff */
	p_ptr->window |= (PW_OVERHEAD);
}



/*
 * This routine clears the entire "temp" set.
 *
 * This routine will Perma-Lite all "temp" grids.
 *
 * This routine is used (only) by "lite_room()"
 *
 * Dark grids are illuminated.
 *
 * Also, process all affected monsters.
 *
 * SMART monsters always wake up when illuminated
 * NORMAL monsters wake up 1/4 the time when illuminated
 * STUPID monsters wake up 1/10 the time when illuminated
 */
static void cave_temp_room_lite(void)
{
	int i;

	/* Clear them all */
	for (i = 0; i < temp_n; i++)
	{
		int y = temp_y[i];
		int x = temp_x[i];

		cave_type *c_ptr = &cave[y][x];

		/* No longer in the array */
		c_ptr->info &= ~(CAVE_TEMP);

		/* Update only non-CAVE_GLOW grids */
		/* if (c_ptr->info & (CAVE_GLOW)) continue; */

		/* Perma-Lite */
		c_ptr->info |= (CAVE_GLOW);

		/* Process affected monsters */
		if (c_ptr->m_idx)
		{
			int chance = 25;

			monster_type    *m_ptr = &m_list[c_ptr->m_idx];

                        monster_race    *r_ptr = race_inf(m_ptr);

			/* Update the monster */
			update_mon(c_ptr->m_idx, FALSE);

			/* Stupid monsters rarely wake up */
			if (r_ptr->flags2 & (RF2_STUPID)) chance = 10;

			/* Smart monsters always wake up */
			if (r_ptr->flags2 & (RF2_SMART)) chance = 100;

			/* Sometimes monsters wake up */
			if (m_ptr->csleep && (rand_int(100) < chance))
			{
				/* Wake up! */
				m_ptr->csleep = 0;

				/* Notice the "waking up" */
				if (m_ptr->ml)
				{
					char m_name[80];

					/* Acquire the monster name */
					monster_desc(m_name, m_ptr, 0);

					/* Dump a message */
					msg_format("%^s wakes up.", m_name);
				}
			}
		}

		/* Note */
		note_spot(y, x);

		/* Redraw */
		lite_spot(y, x);
	}

	/* None left */
	temp_n = 0;
}



/*
 * This routine clears the entire "temp" set.
 *
 * This routine will "darken" all "temp" grids.
 *
 * In addition, some of these grids will be "unmarked".
 *
 * This routine is used (only) by "unlite_room()"
 *
 * Also, process all affected monsters
 */
static void cave_temp_room_unlite(void)
{
	int i;

	/* Clear them all */
	for (i = 0; i < temp_n; i++)
	{
		int y = temp_y[i];
		int x = temp_x[i];

		cave_type *c_ptr = &cave[y][x];

		/* No longer in the array */
		c_ptr->info &= ~(CAVE_TEMP);

		/* Darken the grid */
		c_ptr->info &= ~(CAVE_GLOW);

		/* Hack -- Forget "boring" grids */
		if (c_ptr->feat == FEAT_FLOOR)
		{
			/* Forget the grid */
			c_ptr->info &= ~(CAVE_MARK);

			/* Notice */
			note_spot(y, x);
		}

		/* Process affected monsters */
		if (c_ptr->m_idx)
		{
			/* Update the monster */
			update_mon(c_ptr->m_idx, FALSE);
		}

		/* Redraw */
		lite_spot(y, x);
	}

	/* None left */
	temp_n = 0;
}




/*
 * Aux function -- see below
 */
static void cave_temp_room_aux(int y, int x)
{
	cave_type *c_ptr = &cave[y][x];

	/* Avoid infinite recursion */
	if (c_ptr->info & (CAVE_TEMP)) return;

	/* Do not "leave" the current room */
	if (!(c_ptr->info & (CAVE_ROOM))) return;

	/* Paranoia -- verify space */
	if (temp_n == TEMP_MAX) return;

	/* Mark the grid as "seen" */
	c_ptr->info |= (CAVE_TEMP);

	/* Add it to the "seen" set */
	temp_y[temp_n] = y;
	temp_x[temp_n] = x;
	temp_n++;
}




/*
 * Illuminate any room containing the given location.
 */
void lite_room(int y1, int x1)
{
	int i, x, y;

	/* Add the initial grid */
	cave_temp_room_aux(y1, x1);

	/* While grids are in the queue, add their neighbors */
	for (i = 0; i < temp_n; i++)
	{
		x = temp_x[i], y = temp_y[i];

		/* Walls get lit, but stop light */
		if (!cave_floor_bold(y, x)) continue;

		/* Spread adjacent */
		cave_temp_room_aux(y + 1, x);
		cave_temp_room_aux(y - 1, x);
		cave_temp_room_aux(y, x + 1);
		cave_temp_room_aux(y, x - 1);

		/* Spread diagonal */
		cave_temp_room_aux(y + 1, x + 1);
		cave_temp_room_aux(y - 1, x - 1);
		cave_temp_room_aux(y - 1, x + 1);
		cave_temp_room_aux(y + 1, x - 1);
	}

	/* Now, lite them all up at once */
	cave_temp_room_lite();
}


/*
 * Darken all rooms containing the given location
 */
void unlite_room(int y1, int x1)
{
	int i, x, y;

	/* Add the initial grid */
	cave_temp_room_aux(y1, x1);

	/* Spread, breadth first */
	for (i = 0; i < temp_n; i++)
	{
		x = temp_x[i], y = temp_y[i];

		/* Walls get dark, but stop darkness */
		if (!cave_floor_bold(y, x)) continue;

		/* Spread adjacent */
		cave_temp_room_aux(y + 1, x);
		cave_temp_room_aux(y - 1, x);
		cave_temp_room_aux(y, x + 1);
		cave_temp_room_aux(y, x - 1);

		/* Spread diagonal */
		cave_temp_room_aux(y + 1, x + 1);
		cave_temp_room_aux(y - 1, x - 1);
		cave_temp_room_aux(y - 1, x + 1);
		cave_temp_room_aux(y + 1, x - 1);
	}

	/* Now, darken them all at once */
	cave_temp_room_unlite();
}



/*
 * Hack -- call light around the player
 * Affect all monsters in the projection radius
 */
bool lite_area(int dam, int rad)
{
	int flg = PROJECT_GRID | PROJECT_KILL;

	/* Hack -- Message */
	if (!p_ptr->blind)
	{
		msg_print("You are surrounded by a white light.");
	}

	/* Hook into the "project()" function */
	(void)project(0, rad, py, px, dam, GF_LITE_WEAK, flg);

	/* Lite up the room */
	lite_room(py, px);

	/* Assume seen */
	return (TRUE);
}


/*
 * Hack -- call darkness around the player
 * Affect all monsters in the projection radius
 */
bool unlite_area(int dam, int rad)
{
	int flg = PROJECT_GRID | PROJECT_KILL;

	/* Hack -- Message */
	if (!p_ptr->blind)
	{
		msg_print("Darkness surrounds you.");
	}

	/* Hook into the "project()" function */
	(void)project(0, rad, py, px, dam, GF_DARK_WEAK, flg);

	/* Lite up the room */
	unlite_room(py, px);

	/* Assume seen */
	return (TRUE);
}


/*
 * Cast a ball spell
 * Stop if we hit a monster, act as a "ball"
 * Allow "target" mode to pass over monsters
 * Affect grids, objects, and monsters
 */
bool fire_ball(int typ, int dir, int dam, int rad)
{
	int tx, ty;

	int flg = PROJECT_STOP | PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL;

	/* Use the given direction */
	tx = px + 99 * ddx[dir];
	ty = py + 99 * ddy[dir];

	/* Hack -- Use an actual "target" */
	if ((dir == 5) && target_okay())
	{
		flg &= ~(PROJECT_STOP);
		tx = target_col;
		ty = target_row;
	}

	/* Analyze the "dir" and the "target".  Hurt items on floor. */
        return (project(0, (rad > 16)?16:rad, ty, tx, dam, typ, flg));
}

/*
 * Cast a druidistic ball spell
 * Stop if we hit a monster, act as a "ball"
 * Allow "target" mode to pass over monsters
 * Affect grids, objects, and monsters
 */
bool fire_druid_ball(int typ, int dir, int dam, int rad)
{
	int tx, ty;

        int flg = PROJECT_STOP | PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL | PROJECT_MANA_PATH;

	/* Use the given direction */
	tx = px + 99 * ddx[dir];
	ty = py + 99 * ddy[dir];

	/* Hack -- Use an actual "target" */
	if ((dir == 5) && target_okay())
	{
		flg &= ~(PROJECT_STOP);
		tx = target_col;
		ty = target_row;
	}

	/* Analyze the "dir" and the "target".  Hurt items on floor. */
        return (project(0, (rad > 16)?16:rad, ty, tx, dam, typ, flg));
}


/*
 * Cast a ball-beamed spell
 * Stop if we hit a monster, act as a "ball"
 * Allow "target" mode to pass over monsters
 * Affect grids, objects, and monsters
 */
bool fire_ball_beam(int typ, int dir, int dam, int rad)
{
	int tx, ty;

        int flg = PROJECT_STOP | PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL | PROJECT_BEAM;

	/* Use the given direction */
	tx = px + 99 * ddx[dir];
	ty = py + 99 * ddy[dir];

	/* Hack -- Use an actual "target" */
	if ((dir == 5) && target_okay())
	{
		flg &= ~(PROJECT_STOP);
		tx = target_col;
		ty = target_row;
	}

	/* Analyze the "dir" and the "target".  Hurt items on floor. */
        return (project(0, (rad > 16)?16:rad, ty, tx, dam, typ, flg));
}


void teleport_swap(int dir)
{
	int tx, ty;
	cave_type * c_ptr;
	monster_type * m_ptr;
	monster_race * r_ptr;

        if(p_ptr->resist_continuum) {msg_print("The space-time continuum can't be disrupted."); return;}
	
	if ((dir == 5) && target_okay())
	{
		tx = target_col;
		ty = target_row;
	}
	else
	{
		tx = px + ddx[dir];
		ty = py + ddy[dir];
	}
	c_ptr = &cave[ty][tx];
	
	if (!c_ptr->m_idx)
	{
		msg_print("You can't trade places with that!");
	}
	else
	{
		m_ptr = &m_list[c_ptr->m_idx];
                r_ptr = race_inf(m_ptr);
		
		if (r_ptr->flags3 & RF3_RES_TELE)
		{
			msg_print("Your teleportation is blocked!");
		}
		else
		{
			sound(SOUND_TELEPORT);
			
			cave[py][px].m_idx = c_ptr->m_idx;
			
			/* Update the old location */
			c_ptr->m_idx = 0;
			
			/* Move the monster */
			m_ptr->fy = py;
			m_ptr->fx = px;
			
			/* Move the player */
			px = tx;
			py = ty;
			
			tx = m_ptr->fx;
			ty = m_ptr->fy;
			
			/* Update the monster (new location) */
			update_mon(cave[ty][tx].m_idx, TRUE);
			
			/* Redraw the old grid */
			lite_spot(ty, tx);
			
			/* Redraw the new grid */
			lite_spot(py, px);

                        /* Execute the inscription */
                        c_ptr = &cave[m_ptr->fy][m_ptr->fx];
                        if (c_ptr->inscription)
                        {
                                if(inscription_info[c_ptr->inscription].when & INSCRIP_EXEC_MONST_WALK)
                                {
                                        execute_inscription(c_ptr->inscription, m_ptr->fy, m_ptr->fx);
                                }
                        }
                        c_ptr = &cave[py][px];
                        if (c_ptr->inscription)
                        {
                                msg_format("There is an inscription here: %s", inscription_info[c_ptr->inscription].text);
                                if(inscription_info[c_ptr->inscription].when & INSCRIP_EXEC_WALK)
                                {
                                        execute_inscription(c_ptr->inscription, py, px);
                                }
                        }
			
			/* Check for new panel (redraw map) */
			verify_panel();
			
			/* Update stuff */
			p_ptr->update |= (PU_VIEW | PU_LITE | PU_FLOW);
			
			/* Update the monsters */
			p_ptr->update |= (PU_DISTANCE);
			
			/* Window stuff */
			p_ptr->window |= (PW_OVERHEAD);
			
			/* Handle stuff XXX XXX XXX */
			handle_stuff();
		}
	}
}

void swap_position(int lty, int ltx)
{
        int tx = ltx, ty = lty;
	cave_type * c_ptr;
	monster_type * m_ptr;
	monster_race * r_ptr;

        if(p_ptr->resist_continuum) {msg_print("The space-time continuum can't be disrupted."); return;}
	
	c_ptr = &cave[ty][tx];
	
	if (!c_ptr->m_idx)
	{
			sound(SOUND_TELEPORT);
			
                        /* Keep trace of the old location */
                        tx = px;
                        ty = py;
			
			/* Move the player */
                        px = ltx;
                        py = lty;
			
			/* Redraw the old grid */
			lite_spot(ty, tx);
			
			/* Redraw the new grid */
			lite_spot(py, px);
			
			/* Check for new panel (redraw map) */
			verify_panel();
			
			/* Update stuff */
			p_ptr->update |= (PU_VIEW | PU_LITE | PU_FLOW);
			
			/* Update the monsters */
			p_ptr->update |= (PU_DISTANCE);
			
			/* Window stuff */
			p_ptr->window |= (PW_OVERHEAD);
			
			/* Handle stuff XXX XXX XXX */
			handle_stuff();
	}
	else
	{
		m_ptr = &m_list[c_ptr->m_idx];
                r_ptr = race_inf(m_ptr);
		
			sound(SOUND_TELEPORT);
			
			cave[py][px].m_idx = c_ptr->m_idx;
			
			/* Update the old location */
			c_ptr->m_idx = 0;
			
			/* Move the monster */
			m_ptr->fy = py;
			m_ptr->fx = px;
			
			/* Move the player */
			px = tx;
			py = ty;
			
			tx = m_ptr->fx;
			ty = m_ptr->fy;
			
			/* Update the monster (new location) */
			update_mon(cave[ty][tx].m_idx, TRUE);
			
			/* Redraw the old grid */
			lite_spot(ty, tx);
			
			/* Redraw the new grid */
			lite_spot(py, px);
			
			/* Check for new panel (redraw map) */
			verify_panel();
			
			/* Update stuff */
			p_ptr->update |= (PU_VIEW | PU_LITE | PU_FLOW);
			
			/* Update the monsters */
			p_ptr->update |= (PU_DISTANCE);
			
			/* Window stuff */
			p_ptr->window |= (PW_OVERHEAD);
			
			/* Handle stuff XXX XXX XXX */
			handle_stuff();
	}
}


/*
 * Hack -- apply a "projection()" in a direction (or at the target)
 */
bool project_hook(int typ, int dir, int dam, int flg)
{
	int tx, ty;

	/* Pass through the target if needed */
	flg |= (PROJECT_THRU);

	/* Use the given direction */
	tx = px + ddx[dir];
	ty = py + ddy[dir];

	/* Hack -- Use an actual "target" */
	if ((dir == 5) && target_okay())
	{
		tx = target_col;
		ty = target_row;
	}

	/* Analyze the "dir" and the "target", do NOT explode */
	return (project(0, 0, ty, tx, dam, typ, flg));
}


/*
 * Cast a bolt spell
 * Stop if we hit a monster, as a "bolt"
 * Affect monsters (not grids or objects)
 */
bool fire_bolt(int typ, int dir, int dam)
{
	int flg = PROJECT_STOP | PROJECT_KILL;
	return (project_hook(typ, dir, dam, flg));
}

/*
 * Cast a druidistic bolt spell
 * Stop if we hit a monster, as a "bolt"
 * Affect monsters (not grids or objects)
 */
bool fire_druid_bolt(int typ, int dir, int dam)
{
        int flg = PROJECT_STOP | PROJECT_KILL | PROJECT_MANA_PATH;
	return (project_hook(typ, dir, dam, flg));
}


/*
 * Cast a druidistic beam spell
 * Pass through monsters, as a "beam"
 * Affect monsters (not grids or objects)
 */
bool fire_druid_beam(int typ, int dir, int dam)
{
        int flg = PROJECT_BEAM | PROJECT_KILL | PROJECT_MANA_PATH;
	return (project_hook(typ, dir, dam, flg));
}

/*
 * Cast a beam spell
 * Pass through monsters, as a "beam"
 * Affect monsters (not grids or objects)
 */
bool fire_beam(int typ, int dir, int dam)
{
	int flg = PROJECT_BEAM | PROJECT_KILL;
	return (project_hook(typ, dir, dam, flg));
}


/*
 * Cast a bolt spell, or rarely, a beam spell
 */
bool fire_bolt_or_beam(int prob, int typ, int dir, int dam)
{
	if (rand_int(100) < prob)
	{
		return (fire_beam(typ, dir, dam));
	}
	else
	{
		return (fire_bolt(typ, dir, dam));
	}
}

bool fire_godly_wrath(int y, int x, int typ, int rad, int dam) {
  int flg = PROJECT_STOP | PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL;

  return (project(0, rad, y, x, dam, typ, flg));
}

bool fire_explosion(int y, int x, int typ, int rad, int dam) {
  int flg = PROJECT_STOP | PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL;

  return (project(0, rad, y, x, dam, typ, flg));
}

/*
 * Some of the old functions
 */
bool lite_line(int dir)
{
	int flg = PROJECT_BEAM | PROJECT_GRID | PROJECT_KILL;
	return (project_hook(GF_LITE_WEAK, dir, damroll(6, 8), flg));
}


bool drain_life(int dir, int dam)
{
	int flg = PROJECT_STOP | PROJECT_KILL;
	return (project_hook(GF_OLD_DRAIN, dir, dam, flg));
}


bool wall_to_mud(int dir)
{
	int flg = PROJECT_BEAM | PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL;
	return (project_hook(GF_KILL_WALL, dir, 20 + randint(30), flg));
}


bool wizard_lock(int dir)
{
	int flg = PROJECT_BEAM | PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL;
    return (project_hook(GF_JAM_DOOR, dir, 20 + randint(30), flg));
}


bool destroy_door(int dir)
{
	int flg = PROJECT_BEAM | PROJECT_GRID | PROJECT_ITEM;
	return (project_hook(GF_KILL_DOOR, dir, 0, flg));
}


bool disarm_trap(int dir)
{
	int flg = PROJECT_BEAM | PROJECT_GRID | PROJECT_ITEM;
	return (project_hook(GF_KILL_TRAP, dir, 0, flg));
}


bool heal_monster(int dir)
{
	int flg = PROJECT_STOP | PROJECT_KILL;
	return (project_hook(GF_OLD_HEAL, dir, damroll(4, 6), flg));
}


bool speed_monster(int dir)
{
	int flg = PROJECT_STOP | PROJECT_KILL;
	return (project_hook(GF_OLD_SPEED, dir, p_ptr->lev, flg));
}


bool slow_monster(int dir)
{
	int flg = PROJECT_STOP | PROJECT_KILL;
	return (project_hook(GF_OLD_SLOW, dir, p_ptr->lev, flg));
}


bool sleep_monster(int dir)
{
	int flg = PROJECT_STOP | PROJECT_KILL;
	return (project_hook(GF_OLD_SLEEP, dir, p_ptr->lev, flg));
}


bool stasis_monster(int dir)
{
	int flg = PROJECT_STOP | PROJECT_KILL;
	return (project_hook(GF_STASIS, dir, p_ptr->lev, flg));
}


bool confuse_monster(int dir, int plev)
{
	int flg = PROJECT_STOP | PROJECT_KILL;
	return (project_hook(GF_OLD_CONF, dir, plev, flg));
}


bool stun_monster(int dir, int plev)
{
	int flg = PROJECT_STOP | PROJECT_KILL;
	return (project_hook(GF_STUN, dir, plev, flg));
}


bool poly_monster(int dir)
{
	int flg = PROJECT_STOP | PROJECT_KILL;
	return (project_hook(GF_OLD_POLY, dir, p_ptr->lev, flg));
}


bool clone_monster(int dir)
{
	int flg = PROJECT_STOP | PROJECT_KILL;
	return (project_hook(GF_OLD_CLONE, dir, 0, flg));
}


bool fear_monster(int dir, int plev)
{
	int flg = PROJECT_STOP | PROJECT_KILL;
	return (project_hook(GF_TURN_ALL, dir, plev, flg));
}


bool death_ray(int dir, int plev)
{
	int flg = PROJECT_STOP | PROJECT_KILL;
	return (project_hook(GF_DEATH_RAY, dir, plev, flg));
}


bool teleport_monster(int dir)
{
	int flg = PROJECT_BEAM | PROJECT_KILL;

        if(p_ptr->resist_continuum) {msg_print("The space-time continuum can't be disrupted."); return FALSE;}

	return (project_hook(GF_AWAY_ALL, dir, MAX_SIGHT * 5, flg));
}


/*
 * Hooks -- affect adjacent grids (radius 1 ball attack)
 */
bool door_creation(void)
{
	int flg = PROJECT_GRID | PROJECT_ITEM | PROJECT_HIDE;
	return (project(0, 1, py, px, 0, GF_MAKE_DOOR, flg));
}


bool trap_creation(void)
{
	int flg = PROJECT_GRID | PROJECT_ITEM | PROJECT_HIDE;
	return (project(0, 1, py, px, 0, GF_MAKE_TRAP, flg));
}


bool glyph_creation(void)
{
	int flg = PROJECT_GRID | PROJECT_ITEM;
	return (project(0, 1, py, px, 0, GF_MAKE_GLYPH, flg));
}


bool wall_stone(void)
{
	int flg = PROJECT_GRID | PROJECT_ITEM;

	bool dummy = (project(0, 1, py, px, 0, GF_STONE_WALL, flg));

	/* Update stuff */
	p_ptr->update |= (PU_VIEW | PU_LITE | PU_FLOW);

	/* Update the monsters */
	p_ptr->update |= (PU_MONSTERS);

	/* Redraw map */
	p_ptr->redraw |= (PR_MAP);

	/* Window stuff */
	p_ptr->window |= (PW_OVERHEAD);

	return dummy;
}


bool destroy_doors_touch(void)
{
	int flg = PROJECT_GRID | PROJECT_ITEM | PROJECT_HIDE;
	return (project(0, 1, py, px, 0, GF_KILL_DOOR, flg));
}


bool sleep_monsters_touch(void)
{
	int flg = PROJECT_KILL | PROJECT_HIDE;
	return (project(0, 1, py, px, p_ptr->lev, GF_OLD_SLEEP, flg));
}


void call_chaos(void)
{
	int Chaos_type, dummy, dir;
	int plev = p_ptr->lev;
	bool line_chaos = FALSE;

	int hurt_types[30] =
	{
		GF_ELEC,      GF_POIS,    GF_ACID,    GF_COLD,
		GF_FIRE,      GF_MISSILE, GF_ARROW,   GF_PLASMA,
		GF_HOLY_FIRE, GF_WATER,   GF_LITE,    GF_DARK,
		GF_FORCE,     GF_INERTIA, GF_MANA,    GF_METEOR,
		GF_ICE,       GF_CHAOS,   GF_NETHER,  GF_DISENCHANT,
		GF_SHARDS,    GF_SOUND,   GF_NEXUS,   GF_CONFUSION,
		GF_TIME,      GF_GRAVITY, GF_ROCKET,  GF_NUKE,
		GF_HELL_FIRE, GF_DISINTEGRATE
	};

	Chaos_type = hurt_types[randint(30) - 1];
	if (randint(4) == 1) line_chaos = TRUE;

#if 0
	/* Probably a meaningless line, a remnant from earlier code */
	while (Chaos_type > GF_GRAVITY && Chaos_type < GF_ROCKET);
#endif

	if (randint(6) == 1)
	{
		for (dummy = 1; dummy < 10; dummy++)
		{
			if (dummy-5)
			{
				if (line_chaos)
					fire_beam(Chaos_type, dummy, 75);
				else
					fire_ball(Chaos_type, dummy, 75, 2);
			}
		}
	}
	else if (randint(3)==1)
	{
		fire_ball(Chaos_type, 0, 300, 8);
	}
	else
	{
		if (!get_aim_dir(&dir)) return;
		if (line_chaos)
			fire_beam(Chaos_type, dir, 150);
		else
			fire_ball(Chaos_type, dir, 150, 3 + (plev/35));
	}
}


/*
 * Activate the evil Topi Ylinen curse
 * rr9: Stop the nasty things when a Cyberdemon is summoned
 * or the player gets paralyzed.
 */
void activate_ty_curse(void)
{
	int     i = 0;
	bool    stop_ty = FALSE;

	do
	{
		switch(randint(27))
		{
		case 1: case 2: case 3: case 16: case 17:
			aggravate_monsters(1);
			if (randint(6) != 1) break;
		case 4: case 5: case 6:
			activate_hi_summon();
			if (randint(6) != 1) break;
		case 7: case 8: case 9: case 18:
			(void) summon_specific(py, px, dun_level, 0);
			if (randint(6) != 1) break;
		case 10: case 11: case 12:
			msg_print("You feel your life draining away...");
			lose_exp(p_ptr->exp / 16);
			if (randint(6) != 1) break;
		case 13: case 14: case 15: case 19: case 20:
			if (p_ptr->free_act && (randint(100) < p_ptr->skill_sav))
			{
				/* Do nothing */ ;
			}
			else
			{
				msg_print("You feel like a statue!");
				if (p_ptr->free_act)
					set_paralyzed (p_ptr->paralyzed + randint(3));
				else
					set_paralyzed (p_ptr->paralyzed + randint(13));
				stop_ty = TRUE;
			}
			if (randint(6) != 1) break;
		case 21: case 22: case 23:
			(void)do_dec_stat((randint(6))-1, STAT_DEC_NORMAL);
			if (randint(6) != 1) break;
		case 24:
			msg_print("Huh? Who am I? What am I doing here?");
			lose_all_info();
			break;
		case 25:
			/*
			 * Only summon Cyberdemons deep in the dungeon.
			 */
			if ((dun_level > 65) && !stop_ty)
			{
				summon_cyber();
				stop_ty = TRUE;
				break;
			}
		default:
			while (i<6)
			{
				do
				{
					(void)do_dec_stat(i, STAT_DEC_NORMAL);
				}
				while (randint(2)==1);

				i++;
			}
		}
	}
	while ((randint(3) == 1) && !stop_ty);
}

/*
 * Activate the ultra evil Dark God curse
 */
void activate_dg_curse(void)
{
	int     i = 0;
        bool    stop_dg = FALSE;

	do
	{
                switch(randint(30))
		{
		case 1: case 2: case 3: case 16: case 17:
			aggravate_monsters(1);
                        if (randint(8) != 1) break;
		case 4: case 5: case 6:
                        msg_print("Oh ! You feel that the curse is replicating itself!");
                        curse_equipment_dg(100, 50 * randint(2));
                        if (randint(8) != 1) break;
		case 7: case 8: case 9: case 18:
                        curse_equipment(100, 50 * randint(2));
                        if (randint(8) != 1) break;
		case 10: case 11: case 12:
			msg_print("You feel your life draining away...");
                        lose_exp(p_ptr->exp / 12);
                        if(rand_int(2))
                        {
                                msg_print("You feel the coldness of the black breath attacking you!");
                                p_ptr->black_breath = TRUE;
                        }
                        if (randint(8) != 1) break;
                case 13: case 14: case 15:
			if (p_ptr->free_act && (randint(100) < p_ptr->skill_sav))
			{
				/* Do nothing */ ;
			}
			else
			{
				msg_print("You feel like a statue!");
				if (p_ptr->free_act)
					set_paralyzed (p_ptr->paralyzed + randint(3));
				else
					set_paralyzed (p_ptr->paralyzed + randint(13));
                                stop_dg = TRUE;
			}
                        if (randint(7) != 1) break;
                case 19: case 20:
			{
                                msg_print("Wohhh! you see 10 little Morgoths dancing before you!");
                                set_confused(p_ptr->confused + randint(13 * 2));
                                if(rand_int(2)) stop_dg = TRUE;
			}
                        if (randint(7) != 1) break;
		case 21: case 22: case 23:
                        (void)do_dec_stat((randint(6))-1, STAT_DEC_PERMANENT);
                        if (randint(7) != 1) break;
		case 24:
			msg_print("Huh? Who am I? What am I doing here?");
			lose_all_info();
			break;
                case 27: case 28: case 29:
                        if(inventory[INVEN_WIELD].k_idx)
                        {
                                msg_print("Your weapon now seems useless...");
                                inventory[INVEN_WIELD].art_flags4 = TR4_NEVER_BLOW;
                        }
                        break;
		case 25:
			/*
                         * Only summon Dragon Riders not too shallow in the dungeon.
			 */
                        if ((dun_level > 25) && !stop_dg)
			{
                                msg_print("Oh! You attracted some evil DragonRiders!");
                                summon_dragon_riders();

                                /* This is evil -- DG */
                                if(rand_int(2)) stop_dg = TRUE;
				break;
			}
		default:
			while (i<6)
			{
				do
				{
                                        (void)do_dec_stat(i, STAT_DEC_NORMAL);
				}
				while (randint(2)==1);

				i++;
			}
		}
	}
        while ((randint(4) == 1) && !stop_dg);
}


void activate_hi_summon(void)
{
	int i;

	for (i = 0; i < (randint(9) + (dun_level / 40)); i++)
	{
		switch(randint(26) + (dun_level / 20) )
		{
			case 1: case 2:
				(void) summon_specific(py, px, dun_level, SUMMON_ANT);
				break;
			case 3: case 4:
				(void) summon_specific(py, px, dun_level, SUMMON_SPIDER);
				break;
			case 5: case 6:
				(void) summon_specific(py, px, dun_level, SUMMON_HOUND);
				break;
			case 7: case 8:
				(void) summon_specific(py, px, dun_level, SUMMON_HYDRA);
				break;
			case 9: case 10:
				(void) summon_specific(py, px, dun_level, SUMMON_ANGEL);
				break;
			case 11: case 12:
				(void) summon_specific(py, px, dun_level, SUMMON_UNDEAD);
				break;
			case 13: case 14:
				(void) summon_specific(py, px, dun_level, SUMMON_DRAGON);
				break;
			case 15: case 16:
				(void) summon_specific(py, px, dun_level, SUMMON_DEMON);
				break;
			case 17:
				(void) summon_specific(py, px, dun_level, SUMMON_WRAITH);
				break;
			case 18: case 19:
				(void) summon_specific(py, px, dun_level, SUMMON_UNIQUE);
				break;
			case 20: case 21:
				(void) summon_specific(py, px, dun_level, SUMMON_HI_UNDEAD);
				break;
			case 22: case 23:
				(void) summon_specific(py, px, dun_level, SUMMON_HI_DRAGON);
				break;
			case 24: case 25:
                                (void) summon_specific(py, px, 100, SUMMON_HI_DEMON);
				break;
			default:
				(void) summon_specific(py, px,(((dun_level * 3) / 2) + 5), 0);
		}
	}
}


void summon_cyber(void)
{
	int i;
	int max_cyber = (dun_level / 50) + randint(6);

	for (i = 0; i < max_cyber; i++)
	{
                (void)summon_specific(py, px, 100, SUMMON_HI_DEMON);
	}
}

void summon_dragon_riders()
{
	int i;
        int max_dr = (dun_level / 50) + randint(6);

        for (i = 0; i < max_dr; i++)
	{
                (void)summon_specific(py, px, 100, SUMMON_DRAGONRIDDER);
	}
}


void wall_breaker(void)
{
	int dummy = 5;

	if (randint(80 + p_ptr->lev) < 70)
	{
		do
		{
			dummy = randint(9);
		}
		while ((dummy == 5) || (dummy == 0));

		wall_to_mud (dummy);
	}
	else if (randint(100) > 30)
	{
		/* Prevent destruction of quest levels and town */
		if (!is_quest(dun_level) && dun_level)
			earthquake(py, px, 1);
	}
	else
	{
		for (dummy = 1; dummy < 10; dummy++)
		{
			if (dummy - 5) wall_to_mud(dummy);
		}
	}
}


void bless_weapon(void)
{
	int             item;
	object_type     *o_ptr;
        u32b            f1, f2, f3, f4, f5, esp;
	char            o_name[80];
	cptr            q, s;

	/* Assume enchant weapon */
	item_tester_hook = item_tester_hook_weapon;

	/* Get an item */
	q = "Bless which weapon? ";
	s = "You have weapon to bless.";
	if (!get_item(&item, q, s, (USE_EQUIP | USE_INVEN | USE_FLOOR))) return;

	/* Get the item (in the pack) */
	if (item >= 0)
	{
		o_ptr = &inventory[item];
	}

	/* Get the item (on the floor) */
	else
	{
		o_ptr = &o_list[0 - item];
	}


	/* Description */
	object_desc(o_name, o_ptr, FALSE, 0);

        /* Extract the flags */
        object_flags(o_ptr, &f1, &f2, &f3, &f4, &f5, &esp);

	if (o_ptr->ident & (IDENT_CURSED))
	{

		if (((f3 & (TR3_HEAVY_CURSE)) && (randint (100) < 33)) ||
		    (f3 & (TR3_PERMA_CURSE)))
		{

			msg_format("The black aura on %s %s disrupts the blessing!",
			    ((item >= 0) ? "your" : "the"), o_name);
			return;
		}

		msg_format("A malignant aura leaves %s %s.",
		    ((item >= 0) ? "your" : "the"), o_name);

		/* Uncurse it */
		o_ptr->ident &= ~(IDENT_CURSED);

		/* Hack -- Assume felt */
		o_ptr->ident |= (IDENT_SENSE);

		/* Take note */
                o_ptr->sense = SENSE_UNCURSED;

		/* Recalculate the bonuses */
		p_ptr->update |= (PU_BONUS);

		/* Window stuff */
		p_ptr->window |= (PW_EQUIP);
	}

	/*
	 * Next, we try to bless it. Artifacts have a 1/3 chance of
	 * being blessed, otherwise, the operation simply disenchants
	 * them, godly power negating the magic. Ok, the explanation
	 * is silly, but otherwise priests would always bless every
	 * artifact weapon they find. Ego weapons and normal weapons
	 * can be blessed automatically.
	 */
	if (f3 & TR3_BLESSED)
	{
		msg_format("%s %s %s blessed already.",
		    ((item >= 0) ? "Your" : "The"), o_name,
		    ((o_ptr->number > 1) ? "were" : "was"));
		return;
	}

	if (!(o_ptr->art_name || o_ptr->name1) || (randint(3) == 1))
	{
		/* Describe */
		msg_format("%s %s shine%s!",
		    ((item >= 0) ? "Your" : "The"), o_name,
		    ((o_ptr->number > 1) ? "" : "s"));
		o_ptr->art_flags3 |= TR3_BLESSED;
	}
	else
	{
		bool dis_happened = FALSE;

		msg_print("The artifact resists your blessing!");

		/* Disenchant tohit */
		if (o_ptr->to_h > 0)
		{
			o_ptr->to_h--;
			dis_happened = TRUE;
		}

		if ((o_ptr->to_h > 5) && (rand_int(100) < 33)) o_ptr->to_h--;

		/* Disenchant todam */
		if (o_ptr->to_d > 0)
		{
			o_ptr->to_d--;
			dis_happened = TRUE;
		}

		if ((o_ptr->to_d > 5) && (rand_int(100) < 33)) o_ptr->to_d--;

		/* Disenchant toac */
		if (o_ptr->to_a > 0)
		{
			o_ptr->to_a--;
			dis_happened = TRUE;
		}

		if ((o_ptr->to_a > 5) && (rand_int(100) < 33)) o_ptr->to_a--;

		if (dis_happened)
		{
			msg_print("There is a static feeling in the air...");
			msg_format("%s %s %s disenchanted!",
			    ((item >= 0) ? "Your" : "The"), o_name,
			    ((o_ptr->number > 1) ? "were" : "was"));
		}
	}

	/* Recalculate bonuses */
	p_ptr->update |= (PU_BONUS);

	/* Window stuff */
	p_ptr->window |= (PW_EQUIP | PW_PLAYER);
}


/*
 * Detect all "nonliving", "undead" or "demonic" monsters on current panel
 */
bool detect_monsters_nonliving(void)
{
	int     i, y, x;
	bool    flag = FALSE;

	/* Scan monsters */
	for (i = 1; i < m_max; i++)
	{
		monster_type *m_ptr = &m_list[i];
                monster_race *r_ptr = race_inf(m_ptr);

		/* Skip dead monsters */
		if (!m_ptr->r_idx) continue;

		/* Location */
		y = m_ptr->fy;
		x = m_ptr->fx;

		/* Only detect nearby monsters */
		if (!panel_contains(y, x)) continue;

		/* Detect evil monsters */
		if ((r_ptr->flags3 & (RF3_NONLIVING)) ||
		    (r_ptr->flags3 & (RF3_UNDEAD)) ||
		    (r_ptr->flags3 & (RF3_DEMON)))
		{
			/* Update monster recall window */
			if (monster_race_idx == m_ptr->r_idx)
			{
				/* Window stuff */
				p_ptr->window |= (PW_MONSTER);
			}

			/* Repair visibility later */
			repair_monsters = TRUE;

			/* Hack -- Detect monster */
			m_ptr->mflag |= (MFLAG_MARK | MFLAG_SHOW);

			/* Hack -- See monster */
			m_ptr->ml = TRUE;

			/* Redraw */
			lite_spot(y, x);

			/* Detect */
			flag = TRUE;
		}
	}

	/* Describe */
	if (flag)
	{
		/* Describe result */
		msg_print("You sense the presence of unnatural beings!");
	}

	/* Result */
	return (flag);
}


/*
 * Confuse monsters
 */
bool confuse_monsters(int dam)
{
	return (project_hack(GF_OLD_CONF, dam));
}


/*
 * Charm monsters
 */
bool charm_monsters(int dam)
{
	return (project_hack(GF_CHARM, dam));
}


/*
 * Charm animals
 */
bool charm_animals(int dam)
{
	return (project_hack(GF_CONTROL_ANIMAL, dam));
}

/*
 * Charm demons
 */
bool charm_demons(int dam)
{
	return (project_hack(GF_CONTROL_DEMON, dam));
}


/*
 * Stun monsters
 */
bool stun_monsters(int dam)
{
	return (project_hack(GF_STUN, dam));
}


/*
 * Stasis monsters
 */
bool stasis_monsters(int dam)
{
	return (project_hack(GF_STASIS, dam));
}


/*
 * Mindblast monsters
 */
bool mindblast_monsters(int dam)
{
	return (project_hack(GF_PSI, dam));
}


/*
 * Banish all monsters
 */
bool banish_monsters(int dist)
{
	return (project_hack(GF_AWAY_ALL, dist));
}


/*
 * Turn evil
 */
bool turn_evil(int dam)
{
	return (project_hack(GF_TURN_EVIL, dam));
}


/*
 * Turn everyone
 */
bool turn_monsters(int dam)
{
	return (project_hack(GF_TURN_ALL, dam));
}


/*
 * Death-ray all monsters (note: OBSCENELY powerful)
 */
bool deathray_monsters(void)
{
	return (project_hack(GF_DEATH_RAY, p_ptr->lev));
}


bool charm_monster(int dir, int plev)
{
	int flg = PROJECT_STOP | PROJECT_KILL;
	return (project_hook(GF_CHARM, dir, plev, flg));
}

bool star_charm_monster(int dir, int plev)
{
	int flg = PROJECT_STOP | PROJECT_KILL;
        return (project_hook(GF_STAR_CHARM, dir, plev, flg));
}


bool control_one_undead(int dir, int plev)
{
	int flg = PROJECT_STOP | PROJECT_KILL;
	return (project_hook(GF_CONTROL_UNDEAD, dir, plev, flg));
}


bool charm_animal(int dir, int plev)
{
	int flg = PROJECT_STOP | PROJECT_KILL;
	return (project_hook(GF_CONTROL_ANIMAL, dir, plev, flg));
}

void change_wild_mode(void)
{
        if (p_ptr->immovable && !p_ptr->wild_mode)
	{
                msg_print("Hum, blinking there will take time.");
	}

	p_ptr->wild_mode = !p_ptr->wild_mode;

	/* Leaving */
	p_ptr->leaving = TRUE;
}


void alter_reality(void)
{
	msg_print("The world changes!");

	/* Leaving */
	p_ptr->leaving = TRUE;
}

/* Heal insanity. */
bool heal_insanity(int val) {
  if (p_ptr->csane < p_ptr->msane) {
    p_ptr->csane += val;

    if (p_ptr->csane >= p_ptr->msane) {
      p_ptr->csane = p_ptr->msane;
      p_ptr->csane_frac = 0;
    }

    p_ptr->redraw |= PR_SANITY;
    p_ptr->window |= (PW_SPELL | PW_PLAYER);
    
    if (val < 5) {
      msg_print("You feel a little better.");
    } else if (val < 15) {
      msg_print("You feel better.");
    } else if (val < 35) {
      msg_print("You feel much better.");
    } else {
      msg_print("You feel very good.");
    }

    return TRUE;
  }

  return FALSE;
}

/*
 * Send the player shooting through walls in the given direction until
 * they reach a non-wall space, or a monster, or a permanent wall.
 */
bool passwall(int dir, bool safe)
{
        int x = px, y = py, ox = px, oy = py, lx = px, ly = py;
        cave_type *c_ptr;
        bool ok = FALSE;

        if (p_ptr->wild_mode) return FALSE;
        if (p_ptr->inside_quest) return FALSE;
        if (dungeon_flags1 & LF1_NO_TELEPORT) return FALSE;

        /* Must go somewhere */
        if (dir == 5) return FALSE;

        while (TRUE)
        {
                x += ddx[dir];
                y += ddy[dir];
                c_ptr = &cave[y][x];

                /* Perm walls stops the transfer */
                if ((!in_bounds(y, x)) && (f_info[c_ptr->feat].flags1 & FF1_PERMANENT))
                {
                        /* get the last working position */
                        x -= ddx[dir];
                        y -= ddy[dir];
                        ok = FALSE;
                        break;
                }

                /* Never on a monster */
                if (c_ptr->m_idx) continue;

                /* Never stop in vaults */
                if (c_ptr->info & CAVE_ICKY) continue;

                /* From now on, the location COULD be used in special case */
                lx = x;
                ly = y;

                /* Pass over walls */
                if (f_info[c_ptr->feat].flags1 & FF1_WALL) continue;

                /* So it must be ok */
                ok = TRUE;
                break;
        }

        if (!ok)
        {
                x = lx;
                y = ly;

                if (!safe)
                {
                        msg_print("You emerge in the wall!");
                        take_hit(damroll(10, 8), "becoming one with a wall");
                }
                cave_set_feat(y, x, FEAT_FLOOR);
        }

        /* Redraw the old spot */
        lite_spot(oy, ox);

        /* Move */
        px = x;
        py = y;

	/* Redraw the new spot */
	lite_spot(py, px);

	/* Check for new panel (redraw map) */
	verify_panel();

	/* Update stuff */
	p_ptr->update |= (PU_VIEW | PU_LITE | PU_FLOW);

	/* Update the monsters */
	p_ptr->update |= (PU_DISTANCE);

	/* Window stuff */
	p_ptr->window |= (PW_OVERHEAD);

	/* Handle stuff XXX XXX XXX */
	handle_stuff();

        return (TRUE);
}

bool reset_recall(void)
{
	char buf[80], buf2[80];
	int i,amt;
	bool found;

	found = FALSE;

	while (!found) 
	{
		strcpy(buf,(d_info[p_ptr->recall_dungeon].name+d_name));

		if(!get_string("Which dungeon? ", buf, 80)) return FALSE;

		/* Find the index corresponding to the name */
		for(i = 1; i < max_d_idx; i++)
		{
                        /* Ignore some dungeons */
                        if (d_info[i].flags1 & DF1_NO_RECALL) continue;

			sprintf(buf2, "%s", d_info[i].name + d_name);

			/* Lowercase the name */
			strlower(buf);
			strlower(buf2);
			if(strstr(buf2, buf)) 
			{
				/* valid dungeon found */
				found = TRUE;
				break;
			}
		}
		if (!found) 
		{
			msg_print("Never heard of that dungeon!");
			msg_print(NULL);		
		}
		if (d_info[i].min_plev > p_ptr->lev)
		{
			msg_print("You cannot go there yet!");
			msg_print(NULL);
			found = FALSE;
		}

	}
	amt = get_quantity(format("Which level in %s(%d-%d)? ",d_info[i].name + d_name,
			d_info[i].mindepth,d_info[i].maxdepth),d_info[i].maxdepth);

        if (amt < 1) return FALSE;

	/* Enforce minimum level */
        if (amt < d_info[i].mindepth) (amt = d_info[i].mindepth);

	/* Mega hack -- Forbid levels 99 and 100 */
	if((amt == 99) || (amt == 100)) amt = 98;

	p_ptr->recall_dungeon = i;
	max_dlv[p_ptr->recall_dungeon] = amt;

	return TRUE;
}

/* The only way to get rid of the dreaded DG_CURSE*/
void remove_dg_curse()
{
        int k;

        /* Parse all the items */
        for (k = INVEN_WIELD; k < INVEN_TOTAL; k++)
        {
                object_type *o_ptr = &inventory[k];

                if (o_ptr->k_idx && (o_ptr->art_flags4 & TR4_DG_CURSE))
                {
                        o_ptr->art_flags3 &= ~TR3_HEAVY_CURSE;
                        o_ptr->art_flags3 &= ~TR3_CURSED;
                        o_ptr->art_flags4 &= ~TR4_DG_CURSE;
                        msg_print("The Morgothian Curse withers away.");
                }
        }
}
