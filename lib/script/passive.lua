-- File: passive.lua
-- This file contains the code needed to calculate passive bonus of characters.

-- Returns TRUE if you're only wielding enchanted weapons.
function only_enchanted_weapons ()

	if (not(unarmed())) then

		if (not(get_object_flag4(inven(INVEN_WIELD), TR4_CRAFTED)) and not(inven(INVEN_WIELD).tval == 0)) then
			return FALSE
		end
		if (not(get_object_flag4(inven(INVEN_WIELD+1), TR4_CRAFTED)) and not(inven(INVEN_WIELD+1).tval == 0)) then
			return FALSE
		end
	else
		if (not(get_object_flag4(inven(INVEN_HANDS), TR4_CRAFTED)) and not(inven(INVEN_HANDS).tval == 0)) then
			return FALSE
		end
	end

	if (inven(INVEN_WIELD).tval == 0 and inven(INVEN_WIELD+1).tval == 0 and inven(INVEN_HANDS).tval == 0) then
		return FALSE
	end

	return TRUE

end

function kensai_equip ()


	if (not(shield_has()) and not(heavy_armor()) and not(inven(INVEN_WIELD).tval == TV_RANGED) and not(inven(INVEN_WIELD+1).tval == TV_RANGED)) then 
		if ((inven(INVEN_WIELD).tval == TV_WEAPON) and (inven(INVEN_WIELD).itemskill == 12) and not(get_object_flag4(inven(INVEN_WIELD), TR4_MUST2H))) then
			return TRUE
		end
		if ((inven(INVEN_WIELD+1).tval == TV_WEAPON) and (inven(INVEN_WIELD+1).itemskill == 12) and not(get_object_flag4(inven(INVEN_WIELD+1), TR4_MUST2H))) then
			return TRUE
		end
	end

	return FALSE

end

-- Calculate some passive bonus from the body you incarnate, if morphed or Monster.
function calc_body_bonus ()

        -- If in the player body nothing have to be done
        if (p_ptr.body_monster > 0) then

        	if (p_ptr.disembodied) then
        
                	p_ptr.wraith_form = 20
        	end

        	p_ptr.ac = p_ptr.ac + (p_ptr.lev * 3)
		p_ptr.ac = p_ptr.ac + (p_ptr.abilities[(CLASS_MONSTER * 10) + 6] * 5)
		p_ptr.ac = multiply_divide(p_ptr.ac, m_race(p_ptr.body_monster).ac, 100)
		p_ptr.dis_ac = p_ptr.dis_ac + (p_ptr.lev * 3)
		p_ptr.dis_ac = p_ptr.dis_ac + multiply_divide(p_ptr.dis_ac, p_ptr.lev * 5, 100)
		p_ptr.dis_ac = p_ptr.dis_ac + (p_ptr.abilities[(CLASS_MONSTER * 10) + 6] * 5)
		p_ptr.dis_ac = multiply_divide(p_ptr.dis_ac, m_race(p_ptr.body_monster).ac, 100)

		p_ptr.ac = p_ptr.ac + multiply_divide(p_ptr.ac, p_ptr.abilities[(CLASS_MONSTER * 10) + 6] * 25, 100)
		p_ptr.dis_ac = p_ptr.dis_ac + multiply_divide(p_ptr.dis_ac, p_ptr.abilities[(CLASS_MONSTER * 10) + 6] * 25, 100)
        	p_ptr.pspeed = m_race(p_ptr.body_monster).speed

		if(get_monster_flag7(p_ptr.body_monster, RF7_VERY_FAST)) then p_ptr.pspeed = p_ptr.pspeed + 50 end
		if(get_monster_flag7(p_ptr.body_monster, RF7_FAST)) then p_ptr.pspeed = p_ptr.pspeed + 30 end

        	if(get_monster_flag1(p_ptr.body_monster, RF1_NEVER_MOVE)) then p_ptr.immovable = TRUE end
        	if(get_monster_flag2(p_ptr.body_monster, RF2_STUPID)) then p_ptr.stat_add[A_INT+1] = p_ptr.stat_add[A_INT+1] - 1 end
        	if(get_monster_flag2(p_ptr.body_monster, RF2_SMART)) then p_ptr.stat_add[A_INT+1] = p_ptr.stat_add[A_INT+1] + 1 end
        	if(get_monster_flag2(p_ptr.body_monster, RF2_REFLECTING)) then p_ptr.reflect = TRUE end
        	if(get_monster_flag2(p_ptr.body_monster, RF2_INVISIBLE)) then p_ptr.invis = p_ptr.invis + 20 end
        	if(get_monster_flag2(p_ptr.body_monster, RF2_REGENERATE)) then p_ptr.regenerate = TRUE end
        	if(get_monster_flag2(p_ptr.body_monster, RF2_AURA_FIRE)) then p_ptr.sh_fire = TRUE end
        	if(get_monster_flag2(p_ptr.body_monster, RF2_AURA_ELEC)) then p_ptr.sh_elec = TRUE end
        	if(get_monster_flag2(p_ptr.body_monster, RF2_PASS_WALL)) then p_ptr.wraith_form = 20 end
        	if(get_monster_flag3(p_ptr.body_monster, RF3_NO_FEAR)) then p_ptr.resist_fear = TRUE end
        	if(get_monster_flag3(p_ptr.body_monster, RF3_NO_SLEEP)) then p_ptr.free_act = TRUE end
        	if(get_monster_flag3(p_ptr.body_monster, RF3_NO_CONF)) then p_ptr.resist_conf = TRUE end
        	if(get_monster_flag7(p_ptr.body_monster, RF7_CAN_FLY)) then

			p_ptr.ffall = TRUE
			p_ptr.fly = TRUE
			p_ptr.climb = TRUE
		end
	end
end

-- Calculate the player's max hp.
function calc_hitpoints ()

	local mhp
	local i
	local hpmult
        
	hpmult = 0

        -- You gain a little hp every levels...
        mhp = p_ptr.lev * 5;
        -- You gain 15 hp per points of constitution as well.
	-- Also, every points beyond 5 increase hp by 3%.
        -- The first 4 constitutions points does not give you any bonuses.

        mhp = mhp + ((p_ptr.stat_ind[A_CON+1] - 4) * 15)

	-- Let's give at least 10 hp.
	if (mhp < 10) then mhp = 10 end

	if (p_ptr.stat_ind[A_CON+1] > 5) then

		mhp = mhp + multiply_divide(mhp, (p_ptr.stat_ind[A_CON+1] - 5) * 3, 100)
	end

        -- Make sure we at least have 1 hp
        if (mhp < 1) then mhp = 1 end

	-- Life boosting items.
        i = 24
        while (i < 65) do

                -- Hitpoints multiplier
                if (inven(i).k_idx > 0 and inven(i).disabled == 0) then

                        hpmult = hpmult + inven(i).lifebonus
                end

                i = i + 1
        end

	-- Defender's Heavy Armor Mastery.
	if (p_ptr.abilities[(CLASS_DEFENDER * 10) + 1] >= 1) then

		if (inven(INVEN_BODY).tval == TV_HARD_ARMOR or inven(INVEN_BODY).tval == TV_DRAG_ARMOR) then

			hpmult = hpmult + multiply_divide(inven(INVEN_BODY).ac, p_ptr.abilities[(CLASS_DEFENDER * 10) + 1], 100)
		end
	end

        -- Augment Hitpoint
	mhp = mhp + multiply_divide(mhp, hpmult, 100)
	
	mhp = mhp + multiply_divide(mhp, p_ptr.lev * 10, 100)
	
        i = 0;

        if (p_ptr.disembodied) then mhp = 1 end

	-- New maximum hitpoints
	if (not(p_ptr.mhp == mhp)) then

		-- Enforce maximum
		if (p_ptr.chp >= mhp) then

			p_ptr.chp = mhp
			p_ptr.chp_frac = 0
		end

		-- Save the new max-hitpoints
		p_ptr.mhp = mhp

		-- Update stuff
		lua_update_stuff()
	end

        -- Check the hp...
        if (p_ptr.mhp > 2000000000 or p_ptr.mhp < 0) then p_ptr.mhp = 2000000000 end
end

-- Calculate mana.
function calc_mana ()

	local msp
	local cur_wgt
	local max_wgt
	local i
	local manamult = 0
	local craftbonus
	manamult = 0
	craftbonus = 0

        -- Start with no mana.
        msp = 0
        -- You gain 10 mana per points of intelligence.
        -- The first 5 intelligence points does not give you any bonuses.
        msp = msp + ((p_ptr.stat_ind[A_INT+1] - 5) * 10)

        -- Mage's Wizardry.
        manamult = manamult + (p_ptr.abilities[(CLASS_MAGE * 10) + 1] * 10)

	-- Elemental Lord's Elemental Being.
	manamult = manamult + p_ptr.abilities[(CLASS_ELEM_LORD * 10) + 10] * 2

        -- Spellcraft skill gives you more mana...
        if (p_ptr.skill_base[2] >= 80) then msp = msp + (msp / 4) end

        i = 24
        while (i < 65) do

                -- Mana multiplier
                if (inven(i).k_idx > 0 and inven(i).disabled == 0) then

			-- Rods Mastery.
			if (inven(i).tval == TV_ROD and p_ptr.abilities[(CLASS_MAGE * 10) + 6] >= 1) then

				manamult = manamult + multiply_divide(inven(i).manabonus, p_ptr.abilities[(CLASS_MAGE * 10) + 6] * 20, 100)
			else

                        	manamult = manamult + inven(i).manabonus
			end
                end

		-- Bonus for crafted items if you have the proper ability!
		if (p_ptr.abilities[(CLASS_ENCHANTER * 10) + 5] >= 1 and inven(i).disabled == 0) then

			if (inven(i).tval == TV_WEAPON or inven(i).tval == TV_ROD or inven(i).tval == TV_AMMO) then

				craftbonus = craftbonus + (inven(i).dd * inven(i).ds)
			else
				craftbonus = craftbonus + inven(i).ac
			end
		end

                i = i + 1
        end

	-- Give the proper craft bonus.
	if (craftbonus > 0) then craftbonus = craftbonus + ((craftbonus * (p_ptr.abilities[(CLASS_ENCHANTER * 10) + 5] * 20)) / 100) end

	msp = msp + craftbonus

        -- Augment Mana
	msp = msp + multiply_divide(msp, manamult, 100)

	-- Armor causes encumbrance.
	-- Each equipment parts adds up to a percentile penality based on weight.
	-- For example, wearing an armor that weights 20.0 lb will cause a loss
	-- of 20% of your mana.
	cur_wgt = 0
	if (not(get_object_flag2(inven(INVEN_BODY), TR2_FREE_ACT))) then cur_wgt = cur_wgt + (inven(INVEN_BODY).weight / 10) end
	if (not(get_object_flag2(inven(INVEN_FEET), TR2_FREE_ACT))) then cur_wgt = cur_wgt + (inven(INVEN_FEET).weight / 10) end
	if (not(get_object_flag2(inven(INVEN_HANDS), TR2_FREE_ACT))) then cur_wgt = cur_wgt + (inven(INVEN_HANDS).weight / 10) end
	if (not(get_object_flag2(inven(INVEN_HEAD), TR2_FREE_ACT))) then cur_wgt = cur_wgt + (inven(INVEN_HEAD).weight / 10) end
	if (not(get_object_flag2(inven(INVEN_OUTER), TR2_FREE_ACT))) then cur_wgt = cur_wgt + (inven(INVEN_OUTER).weight / 10) end

	-- Check the mana loss percentage...

	if (cur_wgt > 100) then cur_wgt = 100 end
	if (cur_wgt < 0) then cur_wgt = 0 end

	-- Reduce mana by percentile amount.
	msp = msp - multiply_divide(msp, cur_wgt, 100)

	-- Wearing gloves is especially bad.
	if (inven(INVEN_HANDS).k_idx > 0 and not(get_object_flag2(inven(INVEN_HANDS), TR2_FREE_ACT))) then msp = (msp / 2) end

        -- Check the mana...
        -- Probably will never reach this cap.
        if (msp > 2000000000) then msp = 2000000000 end

	-- Mana can never be negative
	if (msp < 0) then msp = 0 end

	-- New maximum mana
	if (not(p_ptr.msp == msp)) then

		-- Enforce maximum
		if (p_ptr.csp >= msp) then

			p_ptr.csp = msp
			p_ptr.csp_frac = 0
		end

		-- Save the new max-mana
		p_ptr.msp = msp

		-- Update stuff
		lua_update_stuff()
	end

end

function calc_bonuses ()

	local	i
	local	j
	local	v
	local   w
	local	hold
        local   old_invis
	local   old_speed
	local   old_telepathy
	local   old_see_inv
	local   old_dis_ac
	local   old_dis_to_a
	local   extra_blows
	local   extra_shots
	local   intfight = 0
	local   str_bonus = 0
	local   int_bonus = 0
	local   wis_bonus = 0
	local   dex_bonus = 0
	local   con_bonus = 0
	local   chr_bonus = 0
	local   ac_bonus = 0

	-- Save the old speed
	old_speed = p_ptr.pspeed

	-- Save the old vision stuff
	old_telepathy = p_ptr.telepathy
	old_see_inv = p_ptr.see_inv

	-- Save the old armor class
	old_dis_ac = p_ptr.dis_ac
	old_dis_to_a = p_ptr.dis_to_a

        -- Save the old invisibility
        old_invis = p_ptr.invis

	-- Clear extra blows/shots
	extra_blows = 0
	extra_shots = 0

	-- Clear the stat modifiers
	calc_stats(0)

        -- Mana multiplier(obsolete)
        p_ptr.to_m = 0

        -- Spell power
        p_ptr.to_s = 1

	-- Clear the Displayed/Real armor class
	p_ptr.ac = 0
	p_ptr.dis_ac = 0

	-- Clear the Displayed/Real Bonuses
	p_ptr.to_h = 0
	p_ptr.dis_to_h = 0
	p_ptr.to_d = 0
	p_ptr.dis_to_d = 0
	p_ptr.to_a = 0
	p_ptr.dis_to_a = 0

	-- Start with "normal" speed
	p_ptr.pspeed = 110

	-- Start with a single blow per turn
	p_ptr.num_blow = 1

	-- Start with a single blow per turn
	p_ptr.num_blow2 = 1

	-- Start with a single shot per turn
	p_ptr.num_fire = 1

	-- Start with a single shot per turn
	p_ptr.num_fire2 = 1

	-- Reset the "xtra" tval
	p_ptr.tval_xtra = 0

	-- Reset the "ammo" tval
	p_ptr.tval_ammo = 0

	-- Clear all the flags
        p_ptr.invis = 0
	p_ptr.aggravate = FALSE
	p_ptr.teleport = FALSE
	p_ptr.exp_drain = FALSE
	p_ptr.xtra_might = FALSE
	p_ptr.see_inv = FALSE
	p_ptr.free_act = FALSE
	p_ptr.slow_digest = FALSE
	p_ptr.regenerate = FALSE
        p_ptr.fly = FALSE
        p_ptr.climb = FALSE
	p_ptr.ffall = FALSE
	p_ptr.hold_life = FALSE
	p_ptr.telepathy = FALSE
	p_ptr.lite = FALSE
	p_ptr.sustain_str = FALSE
	p_ptr.sustain_int = FALSE
	p_ptr.sustain_wis = FALSE
	p_ptr.sustain_con = FALSE
	p_ptr.sustain_dex = FALSE
	p_ptr.sustain_chr = FALSE
	p_ptr.resist_conf = FALSE
	p_ptr.resist_blind = FALSE
	p_ptr.resist_fear = FALSE
	p_ptr.reflect = FALSE
	p_ptr.sh_fire = FALSE
	p_ptr.sh_elec = FALSE
	p_ptr.wraith_form = 0

	-- Reset resistances.
	calc_resistances(0);
	
        -- Start with no bonuses in skills...
	calc_skills(0)

	-- Base infravision (purely racial)
	p_ptr.see_infra = 0
        
        calc_body_bonus()

	-- Bonuses from classes.
	
	-- General stats bonuses
	p_ptr.stat_add[A_STR+1] = p_ptr.stat_add[A_STR+1] + ((p_ptr.stat_cur[A_STR+1] * classes_def[p_ptr.pclass + 1].str_bonus) / 100)
	p_ptr.stat_add[A_INT+1] = p_ptr.stat_add[A_INT+1] + ((p_ptr.stat_cur[A_INT+1] * classes_def[p_ptr.pclass + 1].int_bonus) / 100)
	p_ptr.stat_add[A_WIS+1] = p_ptr.stat_add[A_WIS+1] + ((p_ptr.stat_cur[A_WIS+1] * classes_def[p_ptr.pclass + 1].wis_bonus) / 100)
	p_ptr.stat_add[A_DEX+1] = p_ptr.stat_add[A_DEX+1] + ((p_ptr.stat_cur[A_DEX+1] * classes_def[p_ptr.pclass + 1].dex_bonus) / 100)
	p_ptr.stat_add[A_CON+1] = p_ptr.stat_add[A_CON+1] + ((p_ptr.stat_cur[A_CON+1] * classes_def[p_ptr.pclass + 1].con_bonus) / 100)
	p_ptr.stat_add[A_CHR+1] = p_ptr.stat_add[A_CHR+1] + ((p_ptr.stat_cur[A_CHR+1] * classes_def[p_ptr.pclass + 1].chr_bonus) / 100)
	
	-- General skills bonuses
	calc_skills(1)

	-- Special bonuses/penalities for some classes.

	if (p_ptr.pclass == CLASS_FIGHTER) then
	  p_ptr.resist_fear = TRUE
	end
	if (p_ptr.pclass == CLASS_PALADIN) then
	  p_ptr.resist_fear = TRUE
	end
	if (p_ptr.pclass == CLASS_MONK) then
	  p_ptr.free_act = TRUE
	end
	if (p_ptr.pclass == CLASS_JUSTICE_WARRIOR) then
	  p_ptr.resistances[GF_LITE+1] = 50
	  p_ptr.hold_life = TRUE
	  p_ptr.resist_fear = TRUE
	  p_ptr.sustain_wis = TRUE
	  p_ptr.sustain_chr = TRUE
	  p_ptr.lite = TRUE
	end
	if (p_ptr.pclass == CLASS_ZELAR) then
	  p_ptr.free_act = TRUE
	  p_ptr.resist_fear = TRUE
	  p_ptr.resist_conf = TRUE
	  p_ptr.sustain_str = TRUE
	  p_ptr.sustain_con = TRUE
	  p_ptr.sustain_wis = TRUE
	end
	if (p_ptr.pclass == CLASS_SOUL_GUARDIAN) then
	  p_ptr.hold_life = TRUE
	  p_ptr.resist_fear = TRUE
	  p_ptr.resist_conf = TRUE
	  p_ptr.sustain_wis = TRUE
	  p_ptr.sustain_chr = TRUE
	end
	if (p_ptr.pclass == CLASS_SHADOW) then
	  p_ptr.sustain_dex = TRUE
	end
	if (p_ptr.pclass == CLASS_KENSAI) then
	  p_ptr.free_act = TRUE
	  p_ptr.resist_fear = TRUE
	  p_ptr.resist_conf = TRUE
	  p_ptr.sustain_wis = TRUE
	end

	-- Bonuses from races
	if (p_ptr.prace == RACE_HUMAN) then
	  p_ptr.stat_add[A_INT+1] = p_ptr.stat_add[A_INT+1] + (p_ptr.stat_cur[A_INT+1] / 4)
	  p_ptr.stat_add[A_CHR+1] = p_ptr.stat_add[A_CHR+1] + (p_ptr.stat_cur[A_CHR+1] / 4)
	  p_ptr.skill_bonus[1] = p_ptr.skill_bonus[1] + (p_ptr.skill_base[1] / 4)
	  p_ptr.skill_bonus[2] = p_ptr.skill_bonus[2] + (p_ptr.skill_base[2] / 4)
	  p_ptr.skill_bonus[3] = p_ptr.skill_bonus[3] + (p_ptr.skill_base[3] / 4)
	  p_ptr.skill_bonus[10] = p_ptr.skill_bonus[10] + (p_ptr.skill_base[10] / 4)
	  p_ptr.skill_bonus[11] = p_ptr.skill_bonus[11] + (p_ptr.skill_base[11] / 4)
	  p_ptr.skill_bonus[12] = p_ptr.skill_bonus[12] + (p_ptr.skill_base[12] / 4)
	  p_ptr.skill_bonus[26] = p_ptr.skill_bonus[26] + (p_ptr.skill_base[26] / 4)
	  p_ptr.skill_bonus[29] = p_ptr.skill_bonus[29] + (p_ptr.skill_base[29] / 4)
	end
	if (p_ptr.prace == RACE_HALF_ELF) then
	  p_ptr.stat_add[A_DEX+1] = p_ptr.stat_add[A_DEX+1] + (p_ptr.stat_cur[A_DEX+1] / 10)
	  p_ptr.stat_add[A_WIS+1] = p_ptr.stat_add[A_WIS+1] + (p_ptr.stat_cur[A_WIS+1] / 10)
	  p_ptr.stat_add[A_INT+1] = p_ptr.stat_add[A_INT+1] + (p_ptr.stat_cur[A_INT+1] / 10)
	  p_ptr.stat_add[A_CHR+1] = p_ptr.stat_add[A_CHR+1] + (p_ptr.stat_cur[A_CHR+1] / 10)

	  p_ptr.skill_bonus[20] = p_ptr.skill_bonus[20] + (p_ptr.skill_base[20] / 10)
	  p_ptr.skill_bonus[6] = p_ptr.skill_bonus[6] + (p_ptr.skill_base[6] / 10)
	  p_ptr.skill_bonus[2] = p_ptr.skill_bonus[2] + (p_ptr.skill_base[2] / 4)
	  p_ptr.skill_bonus[19] = p_ptr.skill_bonus[19] + (p_ptr.skill_base[19] / 4)

	  p_ptr.skill_bonus[1] = p_ptr.skill_bonus[1] + (p_ptr.skill_base[1] / 10)
	  p_ptr.skill_bonus[3] = p_ptr.skill_bonus[3] + (p_ptr.skill_base[3] / 10)
	  p_ptr.skill_bonus[10] = p_ptr.skill_bonus[10] + (p_ptr.skill_base[10] / 10)
	  p_ptr.skill_bonus[11] = p_ptr.skill_bonus[11] + (p_ptr.skill_base[11] / 10)
	  p_ptr.skill_bonus[12] = p_ptr.skill_bonus[12] + (p_ptr.skill_base[12] / 10)
	  p_ptr.skill_bonus[25] = p_ptr.skill_bonus[25] + (p_ptr.skill_base[25] / 10)
	  p_ptr.skill_bonus[26] = p_ptr.skill_bonus[26] + (p_ptr.skill_base[26] / 10)
	end
	if (p_ptr.prace == RACE_ELF) then
	  p_ptr.stat_add[A_DEX+1] = p_ptr.stat_add[A_DEX+1] + (p_ptr.stat_cur[A_DEX+1] / 4)
	  p_ptr.stat_add[A_WIS+1] = p_ptr.stat_add[A_WIS+1] + (p_ptr.stat_cur[A_WIS+1] / 4)
	  p_ptr.stat_add[A_STR+1] = p_ptr.stat_add[A_STR+1] - (p_ptr.stat_cur[A_STR+1] / 4)
	  p_ptr.skill_bonus[20] = p_ptr.skill_bonus[20] + (p_ptr.skill_base[20] / 4)
	  p_ptr.skill_bonus[6] = p_ptr.skill_bonus[6] + (p_ptr.skill_base[6] / 4)
	  p_ptr.skill_bonus[2] = p_ptr.skill_bonus[2] + (p_ptr.skill_base[2] / 4)
	  p_ptr.skill_bonus[9] = p_ptr.skill_bonus[9] + (p_ptr.skill_base[9] / 4)
	  p_ptr.skill_bonus[25] = p_ptr.skill_bonus[25] + (p_ptr.skill_base[25] / 4)
	end
	if (p_ptr.prace == RACE_DWARF) then
	  p_ptr.stat_add[A_STR+1] = p_ptr.stat_add[A_STR+1] + (p_ptr.stat_cur[A_STR+1] / 4)
	  p_ptr.stat_add[A_CON+1] = p_ptr.stat_add[A_CON+1] + (p_ptr.stat_cur[A_CON+1] / 3)
	  p_ptr.stat_add[A_WIS+1] = p_ptr.stat_add[A_WIS+1] - (p_ptr.stat_cur[A_WIS+1] / 4)
	  p_ptr.stat_add[A_CHR+1] = p_ptr.stat_add[A_CHR+1] - (p_ptr.stat_cur[A_CHR+1] / 4)
	  p_ptr.skill_bonus[17] = p_ptr.skill_bonus[17] + (p_ptr.skill_base[17] / 4)
	  p_ptr.skill_bonus[5] = p_ptr.skill_bonus[5] + (p_ptr.skill_base[5] / 4)
	  p_ptr.skill_bonus[30] = p_ptr.skill_bonus[30] + (p_ptr.skill_base[30] / 4)
	  p_ptr.skill_bonus[12] = p_ptr.skill_bonus[12] + (p_ptr.skill_base[12] / 4)
	  p_ptr.skill_bonus[6] = p_ptr.skill_bonus[6] - (p_ptr.skill_base[6] / 4)
	  p_ptr.skill_bonus[2] = p_ptr.skill_bonus[2] - (p_ptr.skill_base[2] / 4)
	  p_ptr.pspeed = p_ptr.pspeed - 1
	end
	if (p_ptr.prace == RACE_GNOME) then
	  p_ptr.stat_add[A_INT+1] = p_ptr.stat_add[A_INT+1] + (p_ptr.stat_cur[A_INT+1] / 4)
	  p_ptr.stat_add[A_CON+1] = p_ptr.stat_add[A_CON+1] + (p_ptr.stat_cur[A_CON+1] / 10)
	  p_ptr.stat_add[A_DEX+1] = p_ptr.stat_add[A_DEX+1] - (p_ptr.stat_cur[A_DEX+1] / 4)
	  p_ptr.skill_bonus[2] = p_ptr.skill_bonus[2] + (p_ptr.skill_base[2] / 4)
	  p_ptr.skill_bonus[11] = p_ptr.skill_bonus[11] + (p_ptr.skill_base[11] / 4)
	  p_ptr.skill_bonus[6] = p_ptr.skill_bonus[6] - (p_ptr.skill_base[6] / 4)
	  p_ptr.skill_bonus[30] = p_ptr.skill_bonus[30] + (p_ptr.skill_base[30] / 10)
	end
	if (p_ptr.prace == RACE_KOBOLD) then
	  p_ptr.stat_add[A_DEX+1] = p_ptr.stat_add[A_DEX+1] + (p_ptr.stat_cur[A_DEX+1] / 4)
	  p_ptr.skill_bonus[21] = p_ptr.skill_bonus[21] + (p_ptr.skill_base[21] / 4)
	  p_ptr.skill_bonus[6] = p_ptr.skill_bonus[6] + (p_ptr.skill_base[6] / 4)
	  p_ptr.skill_bonus[7] = p_ptr.skill_bonus[7] + (p_ptr.skill_base[7] / 4)
	  p_ptr.skill_bonus[10] = p_ptr.skill_bonus[10] + (p_ptr.skill_base[10] / 4)
	  p_ptr.skill_bonus[16] = p_ptr.skill_bonus[16] + (p_ptr.skill_base[16] / 4)
	  p_ptr.resistances[GF_POIS+1] = p_ptr.resistances[GF_POIS+1] + 75
	end
	if (p_ptr.prace == RACE_DEVLING) then
	  p_ptr.stat_add[A_DEX+1] = p_ptr.stat_add[A_DEX+1] + (p_ptr.stat_cur[A_DEX+1] / 4)
	  p_ptr.skill_bonus[15] = p_ptr.skill_bonus[15] + (p_ptr.skill_base[15] / 4)
	  p_ptr.skill_bonus[6] = p_ptr.skill_bonus[6] + p_ptr.skill_base[6]
	  p_ptr.skill_bonus[10] = p_ptr.skill_bonus[10] + (p_ptr.skill_base[10] / 4)
	  p_ptr.resistances[GF_FIRE+1] = p_ptr.resistances[GF_FIRE+1] + 25
	  p_ptr.resistances[GF_DARK+1] = p_ptr.resistances[GF_DARK+1] + 25
	  p_ptr.resistances[GF_LITE+1] = p_ptr.resistances[GF_LITE+1] - 25
	end
	if (p_ptr.prace == RACE_CELESTIAL) then
	  p_ptr.stat_add[A_WIS+1] = p_ptr.stat_add[A_WIS+1] + (p_ptr.stat_cur[A_WIS+1] / 4)
	  p_ptr.stat_add[A_CHR+1] = p_ptr.stat_add[A_CHR+1] + (p_ptr.stat_cur[A_CHR+1] / 4)
	  p_ptr.resistances[GF_LITE+1] = p_ptr.resistances[GF_LITE+1] + 100
	  p_ptr.resistances[GF_ELEC+1] = p_ptr.resistances[GF_ELEC+1] + 25
	  p_ptr.resistances[GF_WIND+1] = p_ptr.resistances[GF_WIND+1] + 25
	  p_ptr.resistances[GF_DARK+1] = p_ptr.resistances[GF_DARK+1] - 100
	  p_ptr.skill_bonus[25] = p_ptr.skill_bonus[25] + (p_ptr.skill_base[25] / 4)
	  p_ptr.skill_bonus[27] = p_ptr.skill_bonus[27] + (p_ptr.skill_base[27] / 4)
	  p_ptr.fly = TRUE
	end
	if (p_ptr.prace == RACE_DEMON) then
	  p_ptr.stat_add[A_INT+1] = p_ptr.stat_add[A_INT+1] + (p_ptr.stat_cur[A_INT+1] / 4)
	  p_ptr.stat_add[A_CHR+1] = p_ptr.stat_add[A_CHR+1] + (p_ptr.stat_cur[A_CHR+1] / 4)
	  p_ptr.resistances[GF_DARK+1] = p_ptr.resistances[GF_DARK+1] + 100
	  p_ptr.resistances[GF_FIRE+1] = p_ptr.resistances[GF_FIRE+1] + 25
	  p_ptr.resistances[GF_COLD+1] = p_ptr.resistances[GF_COLD+1] + 25
	  p_ptr.resistances[GF_LITE+1] = p_ptr.resistances[GF_LITE+1] - 100
	  p_ptr.skill_bonus[23] = p_ptr.skill_bonus[23] + (p_ptr.skill_base[23] / 4)
	  p_ptr.skill_bonus[24] = p_ptr.skill_bonus[24] + (p_ptr.skill_base[24] / 4)
	  p_ptr.fly = TRUE
	end
	if (p_ptr.prace == RACE_ZULGOR) then
	  p_ptr.stat_add[A_STR+1] = p_ptr.stat_add[A_STR+1] + p_ptr.stat_cur[A_STR+1]
	  p_ptr.stat_add[A_INT+1] = p_ptr.stat_add[A_INT+1] + (p_ptr.stat_cur[A_INT+1] / 4)
	  p_ptr.skill_bonus[1] = p_ptr.skill_bonus[1] + (p_ptr.skill_base[1] / 3)
	  p_ptr.resistances[GF_CHAOS+1] = p_ptr.resistances[GF_CHAOS+1] + 100
	end
	if (p_ptr.prace == RACE_MONSTER) then
	  if (get_monster_flag3(p_ptr.body_monster, RF3_NO_STUN) or get_monster_flag3(p_ptr.body_monster, RF3_NO_SLEEP)) then
		p_ptr.paralyzed = 0
		p_ptr.stun = 0
	  end
          if (get_monster_flag3(p_ptr.body_monster, RF3_NO_FEAR)) then p_ptr.resist_fear = TRUE end
	  if (get_monster_flag3(p_ptr.body_monster, RF3_NO_CONF)) then p_ptr.resist_conf = TRUE end
	  p_ptr.pspeed = p_ptr.pspeed + (p_ptr.lev / 5)
          p_ptr.lite = TRUE
	  p_ptr.climb = TRUE
	end

        -- Misc stuff
	
	-- Calculate final resistances!
	-- Includes any previous bonuses/penalities
	calc_resistances(1)

	-- Calculate Misfortune.
	calc_cursed()

	-- Misfortune Embrace may increase Misfortune even more.
	if ((p_ptr.abilities[(CLASS_NIGHT1 * 10) + 9] >= 1) and (p_ptr.alignment < 5)) then

		local misbonus
		misbonus = ((p_ptr.alignment + 5) * (-1)) * p_ptr.abilities[(CLASS_NIGHT1 * 10) + 9]

		p_ptr.cursed = p_ptr.cursed + misbonus
	end

        -- Stat boosting abilities
	-- As of 0.5, each stats has it's own section.

	-- Monsters can gain bonus to many stats.
	if (p_ptr.abilities[(CLASS_MONSTER * 10) + 1] >= 1 or p_ptr.abilities[(CLASS_MONSTER * 10) + 2] >= 1) then

		local greatgains
		local smallgains
		local smallcalc
		local totalgains

		smallcalc = (p_ptr.lev - m_race(p_ptr.body_monster).level) + 1
		if (smallcalc < 0) then smallcalc = 0 end

		greatgains = m_race(p_ptr.body_monster).level * p_ptr.abilities[(CLASS_MONSTER * 10) + 1]
		smallgains = smallcalc * p_ptr.abilities[(CLASS_MONSTER * 10) + 2]

		totalgains = greatgains + smallgains

		str_bonus = totalgains
		int_bonus = totalgains
		wis_bonus = totalgains
		dex_bonus = totalgains
	end

	-- STRENGTH

        -- Balanced Warrior
        if (p_ptr.abilities[(CLASS_WARRIOR * 10) + 3] >= 1) then

		str_bonus = str_bonus + (p_ptr.abilities[(CLASS_WARRIOR * 10) + 3] * 10)
        end

	-- Overwhelming Strength
        if (p_ptr.abilities[(CLASS_FIGHTER * 10) + 5] >= 1) then

		str_bonus = str_bonus + (p_ptr.abilities[(CLASS_FIGHTER * 10) + 5] * 20)
        end

	-- INTELLIGENCE

	-- Wizardry.
        if (p_ptr.abilities[(CLASS_MAGE * 10) + 1] >= 1) then

		int_bonus = int_bonus + (p_ptr.abilities[(CLASS_MAGE * 10) + 1] * 10)
        end

	-- WiSDOM

	-- Perfect Union.
        if (p_ptr.abilities[(CLASS_MONK * 10) + 10] >= 1) then

		wis_bonus = wis_bonus + (p_ptr.abilities[(CLASS_MONK * 10) + 10] * 10)
        end

	-- DEXTERITY

	-- Sharpshooter
        if (p_ptr.abilities[(CLASS_MARKSMAN * 10) + 8] >= 1) then

		dex_bonus = dex_bonus + (p_ptr.abilities[(CLASS_MARKSMAN * 10) + 8] * 10)
        end

	-- CONSTITUTION

	-- Balanced Warrior
        if (p_ptr.abilities[(CLASS_WARRIOR * 10) + 3] >= 1) then

		con_bonus = con_bonus + (p_ptr.abilities[(CLASS_WARRIOR * 10) + 3] * 10)
        end

	-- Ultra Defenses
	if (p_ptr.abilities[(CLASS_DEFENDER * 10) + 9] >= 1) then

		con_bonus = con_bonus + (p_ptr.abilities[(CLASS_DEFENDER * 10) + 9] * 15)
        end

	-- CHARISMA

	-- Inspire Courage.
	if (p_ptr.abilities[(CLASS_BARD * 10) + 2] >= 1) then

		chr_bonus = chr_bonus + (p_ptr.abilities[(CLASS_BARD * 10) + 2] * 5)
	end

	-- Charismatic Musician.
	if (p_ptr.abilities[(CLASS_BARD * 10) + 6] >= 1) then

		chr_bonus = chr_bonus + (p_ptr.abilities[(CLASS_BARD * 10) + 6] * 5)
	end

	-- Bardic Reputation.
	if (p_ptr.abilities[(CLASS_BARD * 10) + 7] >= 1) then

		chr_bonus = chr_bonus + (p_ptr.abilities[(CLASS_BARD * 10) + 7] * 10)
	end

	-- Bardic Grandeur.
	if (p_ptr.abilities[(CLASS_BARD * 10) + 10] >= 1) then

		chr_bonus = chr_bonus + (p_ptr.abilities[(CLASS_BARD * 10) + 10] * 10)
	end

	-- Monster Racial Champion.
	if (p_ptr.abilities[(CLASS_MONSTER * 10) + 9] >= 1) then

		local chrgains

		chrgains = m_race(p_ptr.body_monster).level * p_ptr.abilities[(CLASS_MONSTER * 10) + 9]

		chr_bonus = chr_bonus + chrgains
	end

	-- MULTIPLE STATS

	-- Elemental Lord's Elemental Being.
	if (p_ptr.abilities[(CLASS_ELEM_LORD * 10) + 10] >= 1) then

		local allgains

		allgains = p_ptr.abilities[(CLASS_ELEM_LORD * 10) + 10] * 5

		str_bonus = str_bonus + allgains
		int_bonus = int_bonus + allgains
		con_bonus = con_bonus + allgains
	end


	p_ptr.stat_add[A_STR+1] = p_ptr.stat_add[A_STR+1] + multiply_divide(p_ptr.stat_cur[A_STR+1], str_bonus, 100)
	p_ptr.stat_add[A_INT+1] = p_ptr.stat_add[A_INT+1] + multiply_divide(p_ptr.stat_cur[A_INT+1], int_bonus, 100)
	p_ptr.stat_add[A_WIS+1] = p_ptr.stat_add[A_WIS+1] + multiply_divide(p_ptr.stat_cur[A_WIS+1], wis_bonus, 100)
	p_ptr.stat_add[A_DEX+1] = p_ptr.stat_add[A_DEX+1] + multiply_divide(p_ptr.stat_cur[A_DEX+1], dex_bonus, 100)
	p_ptr.stat_add[A_CON+1] = p_ptr.stat_add[A_CON+1] + multiply_divide(p_ptr.stat_cur[A_CON+1], con_bonus, 100)
	p_ptr.stat_add[A_CHR+1] = p_ptr.stat_add[A_CHR+1] + multiply_divide(p_ptr.stat_cur[A_CHR+1], chr_bonus, 100)

	-- SKILLS BONUS --
	-- Improved Combat Feats.
	if (p_ptr.abilities[(CLASS_WARRIOR * 10) + 6] >= 1) then

		p_ptr.skill_bonus[8] = p_ptr.skill_bonus[8] + multiply_divide(p_ptr.skill_base[8], p_ptr.abilities[(CLASS_WARRIOR * 10) + 6] * 50, 100)
        end
	-- Weapons Mastery.
	if (p_ptr.abilities[(CLASS_WARRIOR * 10) + 10] >= 1) then

		p_ptr.skill_bonus[13] = p_ptr.skill_bonus[13] + multiply_divide(p_ptr.skill_base[13], p_ptr.abilities[(CLASS_WARRIOR * 10) + 10] * 10, 100)
		p_ptr.skill_bonus[14] = p_ptr.skill_bonus[14] + multiply_divide(p_ptr.skill_base[14], p_ptr.abilities[(CLASS_WARRIOR * 10) + 10] * 10, 100)
		p_ptr.skill_bonus[15] = p_ptr.skill_bonus[15] + multiply_divide(p_ptr.skill_base[15], p_ptr.abilities[(CLASS_WARRIOR * 10) + 10] * 10, 100)
		p_ptr.skill_bonus[16] = p_ptr.skill_bonus[16] + multiply_divide(p_ptr.skill_base[16], p_ptr.abilities[(CLASS_WARRIOR * 10) + 10] * 10, 100)
		p_ptr.skill_bonus[17] = p_ptr.skill_bonus[17] + multiply_divide(p_ptr.skill_base[17], p_ptr.abilities[(CLASS_WARRIOR * 10) + 10] * 10, 100)
		p_ptr.skill_bonus[30] = p_ptr.skill_bonus[30] + multiply_divide(p_ptr.skill_base[30], p_ptr.abilities[(CLASS_WARRIOR * 10) + 10] * 10, 100)
        end

	-- Stealthy Fighter.
	if (p_ptr.abilities[(CLASS_ROGUE * 10) + 1] >= 1 and not(inven(INVEN_BODY).tval == TV_HARD_ARMOR or inven(INVEN_BODY).tval == TV_DRAG_ARMOR)) then

		p_ptr.skill_bonus[7] = p_ptr.skill_bonus[7] + multiply_divide(p_ptr.skill_base[7], p_ptr.abilities[(CLASS_ROGUE * 10) + 1] * 5, 100)
        end

	-- Blessing of Protection.
	if (p_ptr.abilities[(CLASS_PRIEST * 10) + 3] >= 1 and p_ptr.events[29054] == 1) then

		p_ptr.skill_bonus[5] = p_ptr.skill_bonus[5] + multiply_divide(p_ptr.stat_ind[A_WIS+1], p_ptr.abilities[(CLASS_PRIEST * 10) + 3] * 3, 100)
		p_ptr.skill_bonus[6] = p_ptr.skill_bonus[6] + multiply_divide(p_ptr.stat_ind[A_WIS+1], p_ptr.abilities[(CLASS_PRIEST * 10) + 3] * 3, 100)
		p_ptr.skill_bonus[28] = p_ptr.skill_bonus[28] + multiply_divide(p_ptr.stat_ind[A_WIS+1], p_ptr.abilities[(CLASS_PRIEST * 10) + 3] * 3, 100)
        end

	-- Divine Intervention.
	if (p_ptr.abilities[(CLASS_PRIEST * 10) + 9] >= 1 and p_ptr.events[29056] > 0) then

		p_ptr.skill_bonus[5] = p_ptr.skill_bonus[5] + multiply_divide(p_ptr.stat_ind[A_WIS+1], 100 + (p_ptr.abilities[(CLASS_PRIEST * 10) + 9] * 20), 100)
		p_ptr.skill_bonus[28] = p_ptr.skill_bonus[28] + multiply_divide(p_ptr.stat_ind[A_WIS+1], 100 + (p_ptr.abilities[(CLASS_PRIEST * 10) + 9] * 20), 100)
        end

	-- Inspire Courage
	if (p_ptr.abilities[(CLASS_BARD * 10) + 2] >= 1) then

		p_ptr.skill_bonus[1] = p_ptr.skill_bonus[1] + multiply_divide(p_ptr.stat_ind[A_CHR+1], p_ptr.abilities[(CLASS_BARD * 10) + 2] * 3, 100)
		p_ptr.skill_bonus[2] = p_ptr.skill_bonus[2] + multiply_divide(p_ptr.stat_ind[A_CHR+1], p_ptr.abilities[(CLASS_BARD * 10) + 2] * 3, 100)
		p_ptr.skill_bonus[3] = p_ptr.skill_bonus[3] + multiply_divide(p_ptr.stat_ind[A_CHR+1], p_ptr.abilities[(CLASS_BARD * 10) + 2] * 3, 100)
		p_ptr.skill_bonus[29] = p_ptr.skill_bonus[29] + multiply_divide(p_ptr.stat_ind[A_CHR+1], p_ptr.abilities[(CLASS_BARD * 10) + 2] * 3, 100)
        end

	-- Charismatic Musician.
	if (p_ptr.abilities[(CLASS_BARD * 10) + 6] >= 1) then

		p_ptr.skill_bonus[29] = p_ptr.skill_bonus[29] + multiply_divide(p_ptr.skill_base[29], p_ptr.abilities[(CLASS_BARD * 10) + 6] * 10, 100)
        end

	-- Elemental Being.
	if (p_ptr.abilities[(CLASS_ELEM_LORD * 10) + 10] >= 1) then

		p_ptr.skill_bonus[23] = p_ptr.skill_bonus[23] + multiply_divide(p_ptr.skill_base[23], p_ptr.abilities[(CLASS_ELEM_LORD * 10) + 10] * 5, 100)
        end

	-- Ultra Defenses.
	if (p_ptr.abilities[(CLASS_DEFENDER * 10) + 9] >= 1) then

		p_ptr.skill_bonus[5] = p_ptr.skill_bonus[5] + multiply_divide(p_ptr.skill_base[5], p_ptr.abilities[(CLASS_DEFENDER * 10) + 9] * 15, 100)
		p_ptr.skill_bonus[28] = p_ptr.skill_bonus[28] + multiply_divide(p_ptr.skill_base[28], p_ptr.abilities[(CLASS_DEFENDER * 10) + 9] * 15, 100)
        end


	-- OTHER BONUS --

	-- Lore of the Bard.
	if (p_ptr.abilities[(CLASS_BARD * 10) + 3] >= 1) then

		p_ptr.stat_add[A_INT+1] = p_ptr.stat_add[A_INT+1] + multiply_divide(p_ptr.stat_cur[A_CHR+1], p_ptr.abilities[(CLASS_BARD * 10) + 3] * 10, 100)
		p_ptr.stat_add[A_WIS+1] = p_ptr.stat_add[A_WIS+1] + multiply_divide(p_ptr.stat_cur[A_CHR+1], p_ptr.abilities[(CLASS_BARD * 10) + 3] * 10, 100)
		p_ptr.stat_add[A_CHR+1] = p_ptr.stat_add[A_CHR+1] + multiply_divide(p_ptr.stat_cur[A_CHR+1], p_ptr.abilities[(CLASS_BARD * 10) + 3] * 10, 100)
        end

	-- Elemental Lord's Elemental Spellsword.
	if (p_ptr.abilities[(CLASS_ELEM_LORD * 10) + 1] >= 1) then
        
                p_ptr.stat_add[A_STR+1] = p_ptr.stat_add[A_STR+1] + multiply_divide(p_ptr.stat_cur[A_INT+1], p_ptr.abilities[(CLASS_ELEM_LORD * 10) + 1] * 20, 100)
		p_ptr.stat_add[A_INT+1] = p_ptr.stat_add[A_INT+1] + multiply_divide(p_ptr.stat_cur[A_STR+1], p_ptr.abilities[(CLASS_ELEM_LORD * 10) + 1] * 20, 100)
        end
	-- Elemental Armor's resistance to chosen element.
	if (p_ptr.abilities[(CLASS_ELEM_LORD * 10) + 4] >= 1) then

		p_ptr.resistances[p_ptr.elemlord+1] = p_ptr.resistances[p_ptr.elemlord+1] + (p_ptr.abilities[(CLASS_ELEM_LORD * 10) + 4] * 20)
		if (p_ptr.resistances[p_ptr.elemlord+1] > 100) then p_ptr.resistances[p_ptr.elemlord+1] = 100 end
	end
	-- Elemental Being makes you immune to your chosen element. It also improves Physical resistance.
	if (p_ptr.abilities[(CLASS_ELEM_LORD * 10) + 10] >= 1) then

		local physresbonus

		physresbonus = p_ptr.abilities[(CLASS_ELEM_LORD * 10) + 10] * 2
		if (physresbonus > 75) then physresbonus = 75 end

		p_ptr.resistances[p_ptr.elemlord+1] = 100
		p_ptr.resistances[GF_PHYSICAL+1] = p_ptr.resistances[GF_PHYSICAL+1] + physresbonus
		if (p_ptr.resistances[GF_PHYSICAL+1] > 100) then p_ptr.resistances[GF_PHYSICAL+1] = 100 end
	end

	-- Paragon Elder Monster.
	if (p_ptr.abilities[(CLASS_MONSTER * 10) + 10] >= 1) then

		local statsgains

		statsgains = 10 * p_ptr.abilities[(CLASS_MONSTER * 10) + 10]

		p_ptr.stat_add[A_STR+1] = p_ptr.stat_add[A_STR+1] + multiply_divide(p_ptr.stat_cur[A_STR+1], statsgains, 100)
		p_ptr.stat_add[A_INT+1] = p_ptr.stat_add[A_INT+1] + multiply_divide(p_ptr.stat_cur[A_INT+1], statsgains, 100)
		p_ptr.stat_add[A_WIS+1] = p_ptr.stat_add[A_WIS+1] + multiply_divide(p_ptr.stat_cur[A_WIS+1], statsgains, 100)
		p_ptr.stat_add[A_DEX+1] = p_ptr.stat_add[A_DEX+1] + multiply_divide(p_ptr.stat_cur[A_DEX+1], statsgains, 100)
		p_ptr.stat_add[A_CON+1] = p_ptr.stat_add[A_CON+1] + multiply_divide(p_ptr.stat_cur[A_CON+1], statsgains, 100)
		p_ptr.stat_add[A_CHR+1] = p_ptr.stat_add[A_CHR+1] + multiply_divide(p_ptr.stat_cur[A_CHR+1], statsgains, 100)
	end

	-- Ghostly Misfortune ability!
	if (p_ptr.abilities[(CLASS_NIGHT1 * 10) + 2] > 0) then

		p_ptr.resistances[GF_PHYSICAL+1] = p_ptr.resistances[GF_PHYSICAL+1] + (p_ptr.abilities[(CLASS_NIGHT1 * 10) + 2] * 10)
		if (p_ptr.resistances[GF_PHYSICAL+1] > 100) then p_ptr.resistances[GF_PHYSICAL+1] = 100 end
		if (p_ptr.cursed >= dun_level) then p_ptr.wraith_form = 20 end
	end

   -- Wielding a rod + high rod skill? Raise the spellcraft! ;)
   -- And all other specialized spells skills too!
   if (rod_has() and p_ptr.skill_base[18] >= 70) then

	p_ptr.skill_bonus[2] = p_ptr.skill_bonus[2] + p_ptr.skill_base[2] / 2
	p_ptr.skill_bonus[23] = p_ptr.skill_bonus[23] + p_ptr.skill_base[23] / 2
	p_ptr.skill_bonus[24] = p_ptr.skill_bonus[24] + p_ptr.skill_base[24] / 2
	p_ptr.skill_bonus[25] = p_ptr.skill_bonus[25] + p_ptr.skill_base[25] / 2
	p_ptr.skill_bonus[26] = p_ptr.skill_bonus[26] + p_ptr.skill_base[26] / 2
	p_ptr.skill_bonus[27] = p_ptr.skill_bonus[27] + p_ptr.skill_base[27] / 2
   end

   -- High Mage's Spell Mastery!
   if (p_ptr.abilities[(CLASS_HIGH_MAGE * 10) + 10] >= 1) then

           p_ptr.skill_bonus[2] = p_ptr.skill_bonus[2] + (p_ptr.abilities[(CLASS_HIGH_MAGE * 10) + 10] * 2)
	   p_ptr.skill_bonus[23] = p_ptr.skill_bonus[23] + (p_ptr.abilities[(CLASS_HIGH_MAGE * 10) + 10] * 2)
	   p_ptr.skill_bonus[24] = p_ptr.skill_bonus[24] + (p_ptr.abilities[(CLASS_HIGH_MAGE * 10) + 10] * 2)
	   p_ptr.skill_bonus[25] = p_ptr.skill_bonus[25] + (p_ptr.abilities[(CLASS_HIGH_MAGE * 10) + 10] * 2)
	   p_ptr.skill_bonus[26] = p_ptr.skill_bonus[26] + (p_ptr.abilities[(CLASS_HIGH_MAGE * 10) + 10] * 2)
	   p_ptr.skill_bonus[27] = p_ptr.skill_bonus[27] + (p_ptr.abilities[(CLASS_HIGH_MAGE * 10) + 10] * 2)
   end

   -- Paladin's Champion of Good
   if (p_ptr.abilities[(CLASS_PALADIN * 10) + 10] >= 1) then

	if (p_ptr.alignment > 0) then

		p_ptr.stat_add[A_STR+1] = p_ptr.stat_add[A_STR+1] + (((p_ptr.stat_cur[A_STR+1] * (p_ptr.alignment * 10)) / 100) * p_ptr.abilities[(CLASS_PALADIN * 10) + 10])
		p_ptr.stat_add[A_CON+1] = p_ptr.stat_add[A_CON+1] + (((p_ptr.stat_cur[A_CON+1] * (p_ptr.alignment * 10)) / 100) * p_ptr.abilities[(CLASS_PALADIN * 10) + 10])
		p_ptr.stat_add[A_CHR+1] = p_ptr.stat_add[A_CHR+1] + (((p_ptr.stat_cur[A_CHR+1] * (p_ptr.alignment * 10)) / 100) * p_ptr.abilities[(CLASS_PALADIN * 10) + 10])

        	p_ptr.to_h = p_ptr.to_h + ((20 * p_ptr.alignment) * p_ptr.abilities[(CLASS_PALADIN * 10) + 10])
        	p_ptr.dis_to_h = p_ptr.dis_to_h + ((20 * p_ptr.alignment) * p_ptr.abilities[(CLASS_PALADIN * 10) + 10])
		p_ptr.to_d = p_ptr.to_d + ((20 * p_ptr.alignment) * p_ptr.abilities[(CLASS_PALADIN * 10) + 10])
        	p_ptr.dis_to_d = p_ptr.dis_to_d + ((20 * p_ptr.alignment) * p_ptr.abilities[(CLASS_PALADIN * 10) + 10])
		p_ptr.to_a = p_ptr.to_a + ((50 * p_ptr.alignment) * p_ptr.abilities[(CLASS_PALADIN * 10) + 10])
        	p_ptr.dis_to_a = p_ptr.dis_to_a + ((50 * p_ptr.alignment) * p_ptr.abilities[(CLASS_PALADIN * 10) + 10])
	end
   end

   -- Misfortune Embrace
   if (p_ptr.abilities[(CLASS_NIGHT1 * 10) + 9] >= 1) then

	if (p_ptr.alignment < 0) then

		local sbonus

		sbonus = p_ptr.cursed / 10
		sbonus = sbonus * (p_ptr.alignment * (-1))
		sbonus = sbonus * p_ptr.abilities[(CLASS_NIGHT1 * 10) + 9]

		p_ptr.stat_add[A_STR+1] = p_ptr.stat_add[A_STR+1] + multiply_divide(p_ptr.stat_cur[A_STR+1], sbonus, 100)
		p_ptr.stat_add[A_INT+1] = p_ptr.stat_add[A_INT+1] + multiply_divide(p_ptr.stat_cur[A_INT+1], sbonus, 100)
		p_ptr.stat_add[A_WIS+1] = p_ptr.stat_add[A_WIS+1] + multiply_divide(p_ptr.stat_cur[A_WIS+1], sbonus, 100)
		p_ptr.stat_add[A_DEX+1] = p_ptr.stat_add[A_DEX+1] + multiply_divide(p_ptr.stat_cur[A_DEX+1], sbonus, 100)
		p_ptr.stat_add[A_CON+1] = p_ptr.stat_add[A_CON+1] + multiply_divide(p_ptr.stat_cur[A_CON+1], sbonus, 100)
		p_ptr.stat_add[A_CHR+1] = p_ptr.stat_add[A_CHR+1] + multiply_divide(p_ptr.stat_cur[A_CHR+1], sbonus, 100)
	end
   end

   -- Ranger's Forestry ability!
   if ((p_ptr.abilities[(CLASS_RANGER * 10) + 2] >= 1) and (standing_on_forest())) then

   	p_ptr.to_h = p_ptr.to_h + (p_ptr.abilities[(CLASS_RANGER * 10) + 2] * 30)
        p_ptr.dis_to_h = p_ptr.dis_to_h + (p_ptr.abilities[(CLASS_RANGER * 10) + 2] * 30)
        p_ptr.to_d = p_ptr.to_d + (p_ptr.abilities[(CLASS_RANGER * 10) + 2] * 30)
        p_ptr.dis_to_d = p_ptr.dis_to_d + (p_ptr.abilities[(CLASS_RANGER * 10) + 2] * 30)
        p_ptr.to_a = p_ptr.to_a + (p_ptr.abilities[(CLASS_RANGER * 10) + 2] * 30)
        p_ptr.dis_to_a = p_ptr.dis_to_a + (p_ptr.abilities[(CLASS_RANGER * 10) + 2] * 30)
        p_ptr.pspeed = p_ptr.pspeed + ((p_ptr.abilities[(CLASS_RANGER * 10) + 2]) + 1)
        p_ptr.skill_bonus[7] = p_ptr.skill_bonus[7] + (p_ptr.abilities[(CLASS_RANGER * 10) + 2] * 5)              
   end
   -- Kensai's Honorable Warrior
   if (p_ptr.abilities[(CLASS_KENSAI * 10) + 3] >= 1) then

	if (p_ptr.alignment > 0) then

		p_ptr.stat_add[A_WIS+1] = p_ptr.stat_add[A_WIS+1] + ((p_ptr.stat_cur[A_WIS+1] * (p_ptr.alignment * 5) * (p_ptr.abilities[(CLASS_KENSAI * 10) + 3])) / 100)

	end
   end

   -- High Monk's Legendary Agility!
   if (p_ptr.abilities[(CLASS_ZELAR * 10) + 10] >= 1) then

           p_ptr.skill_bonus[6] = p_ptr.skill_bonus[6] + (p_ptr.abilities[(CLASS_ZELAR * 10) + 10] * 3)
   end

	-- If you have the "SAFETY" ability, you can't be paralyzed!
	if (safety_check()) then

                p_ptr.stun = 0
                p_ptr.paralyzed = 0
        end

	-- Temporary Telepathy
	if (p_ptr.tim_esp > 0) then

		p_ptr.telepathy = TRUE
	end

	-- Temporary see invisible
	if (p_ptr.tim_invis > 0) then

		p_ptr.see_inv = TRUE;
	end

        -- Hack -- Can Fly -> Can Levitate
        if (p_ptr.fly) then

                p_ptr.ffall = TRUE
	end

	-- Hack -- Hero/Shero -> Res fear
	if ((p_ptr.hero > 0) or (p_ptr.shero > 0)) then

		p_ptr.resist_fear = TRUE
	end

	-- Shadow Stalker's Shadow Cloak and One with Shadows!
        if (p_ptr.tim_invisible > 0) then

		if (p_ptr.abilities[(CLASS_SHADOW * 10) + 3] >= 1) then

                        local stealthbonus
			stealthbonus = multiply_divide(p_ptr.skill_base[7], p_ptr.abilities[(CLASS_SHADOW * 10) + 3] * 10, 100)

			p_ptr.skill_bonus[7] = p_ptr.skill_bonus[7] + stealthbonus
                end

                if (p_ptr.abilities[(CLASS_SHADOW * 10) + 8] >= 1) then

                        local statbonus
			statbonus = p_ptr.skill[7] / 4
                        statbonus = statbonus + (statbonus * ((p_ptr.abilities[(CLASS_SHADOW * 10) + 8] * 10) / 100))
                        p_ptr.skill_bonus[6] = p_ptr.skill_bonus[6] + statbonus
                end

		-- Nightmare Ability: Shadow of Misfortune
		if (p_ptr.abilities[(CLASS_NIGHT1 * 10) + 4] >= 1) then

			local statbonus
			statbonus = (p_ptr.cursed / 3)
			if (statbonus < 5) then statbonus = 5 end
			statbonus = statbonus * p_ptr.abilities[(CLASS_NIGHT1 * 10) + 4]
			p_ptr.stat_ind[A_DEX+1] = p_ptr.stat_ind[A_DEX+1] + statbonus
		end
	end

	-- Light Armored Nightmare!
   	if (p_ptr.abilities[(CLASS_NIGHT1 * 10) + 8] >= 1) then

		if (inven(INVEN_BODY).tval == TV_SOFT_ARMOR) then

			local acbonus

			acbonus = 20 * p_ptr.abilities[(CLASS_NIGHT1 * 10) + 8]
			acbonus = acbonus + multiply_divide(acbonus, p_ptr.cursed, 100)

			p_ptr.skill_bonus[5] = p_ptr.skill_bonus[5] + multiply_divide(inven(INVEN_BODY).ac, acbonus, 100)
			p_ptr.skill_bonus[6] = p_ptr.skill_bonus[6] + multiply_divide(inven(INVEN_BODY).ac, acbonus, 100)
			p_ptr.skill_bonus[28] = p_ptr.skill_bonus[28] + multiply_divide(inven(INVEN_BODY).ac, acbonus, 100)
		end
   	end


	-- Hack -- Telepathy Change + See Invis Change 
	-- (adelie) Combined these two hacks to prevent an extra redraw in the event that the player has both see invisibility and telepathy.
	if ((not(p_ptr.telepathy == old_telepathy)) or (not(p_ptr.see_inv == old_see_inv))) then

		lua_update_monsters()
	end

	-- Monster race adjustments.
	-- Percentile adjustments to stats. This applies after everything else.
	if (p_ptr.prace == RACE_MONSTER) then

		if (m_race(p_ptr.body_monster).str < 100) then
			p_ptr.stat_add[A_STR+1] = p_ptr.stat_add[A_STR+1] - multiply_divide(p_ptr.stat_cur[A_STR+1], 100 - m_race(p_ptr.body_monster).str, 100)
		else
			p_ptr.stat_add[A_STR+1] = p_ptr.stat_add[A_STR+1] + multiply_divide(p_ptr.stat_cur[A_STR+1], m_race(p_ptr.body_monster).str - 100, 100)
		end

		if (m_race(p_ptr.body_monster).dex < 100) then
			p_ptr.stat_add[A_DEX+1] = p_ptr.stat_add[A_DEX+1] - multiply_divide(p_ptr.stat_cur[A_DEX+1], 100 - m_race(p_ptr.body_monster).dex, 100)
		else
			p_ptr.stat_add[A_DEX+1] = p_ptr.stat_add[A_DEX+1] + multiply_divide(p_ptr.stat_cur[A_DEX+1], m_race(p_ptr.body_monster).dex - 100, 100)
		end

		if (m_race(p_ptr.body_monster).mind < 100) then
			p_ptr.stat_add[A_INT+1] = p_ptr.stat_add[A_INT+1] - multiply_divide(p_ptr.stat_cur[A_INT+1], 100 - m_race(p_ptr.body_monster).mind, 100)
			p_ptr.stat_add[A_WIS+1] = p_ptr.stat_add[A_WIS+1] - multiply_divide(p_ptr.stat_cur[A_WIS+1], 100 - m_race(p_ptr.body_monster).mind, 100)
		else
			p_ptr.stat_add[A_INT+1] = p_ptr.stat_add[A_INT+1] + multiply_divide(p_ptr.stat_cur[A_INT+1], m_race(p_ptr.body_monster).mind - 100, 100)
			p_ptr.stat_add[A_WIS+1] = p_ptr.stat_add[A_WIS+1] + multiply_divide(p_ptr.stat_cur[A_WIS+1], m_race(p_ptr.body_monster).mind - 100, 100)
		end

		if (m_race(p_ptr.body_monster).hp < 100) then
			p_ptr.stat_add[A_CON+1] = p_ptr.stat_add[A_CON+1] - multiply_divide(p_ptr.stat_cur[A_CON+1], 100 - m_race(p_ptr.body_monster).hp, 100)
		else
			p_ptr.stat_add[A_CON+1] = p_ptr.stat_add[A_CON+1] + multiply_divide(p_ptr.stat_cur[A_CON+1], m_race(p_ptr.body_monster).hp - 100, 100)
		end
	end



	-- Scan the usable inventory
	-- Was once a lua code, now an hard-coded function to increase performances.
	calc_equipment()

	-- We should have the base AC and to_a now.
	-- Apply Defense skill here.

	-- If wearing light or no armor, add a bonus for Martial Arts skill.
	if (not(inven(INVEN_BODY).tval == TV_HARD_ARMOR) and not(inven(INVEN_BODY).tval == TV_DRAG_ARMOR)) then
		p_ptr.ac = p_ptr.ac + (p_ptr.skill[19] / 3)
		p_ptr.dis_ac = p_ptr.dis_ac + (p_ptr.skill[19] / 3)
	end

	-- If wearing hard/dragon armor, get a bonus from Heavy Armor Mastery.
	if ((inven(INVEN_BODY).tval == TV_HARD_ARMOR) or (inven(INVEN_BODY).tval == TV_DRAG_ARMOR)) then

		if (p_ptr.abilities[(CLASS_DEFENDER * 10) + 1] >= 1) then

			p_ptr.ac = p_ptr.ac + multiply_divide(inven(INVEN_BODY).ac, p_ptr.abilities[(CLASS_DEFENDER * 10) + 1] * 3, 100)
			p_ptr.dis_ac = p_ptr.dis_ac + multiply_divide(inven(INVEN_BODY).ac, p_ptr.abilities[(CLASS_DEFENDER * 10) + 1] * 3, 100)
		end
	end

	-- If wielding a shield, get a bonus from Heavy Armor Mastery.
	if (inven(INVEN_WIELD).tval == TV_SHIELD) then

		if (p_ptr.abilities[(CLASS_DEFENDER * 10) + 2] >= 1) then

			p_ptr.ac = p_ptr.ac + multiply_divide(inven(INVEN_WIELD).ac, p_ptr.abilities[(CLASS_DEFENDER * 10) + 2] * 3, 100)
			p_ptr.dis_ac = p_ptr.dis_ac + multiply_divide(inven(INVEN_WIELD).ac, p_ptr.abilities[(CLASS_DEFENDER * 10) + 2] * 3, 100)
		end
	end
	if (inven(INVEN_WIELD+1).tval == TV_SHIELD) then

		if (p_ptr.abilities[(CLASS_DEFENDER * 10) + 2] >= 1) then

			p_ptr.ac = p_ptr.ac + multiply_divide(inven(INVEN_WIELD+1).ac, p_ptr.abilities[(CLASS_DEFENDER * 10) + 2] * 3, 100)
			p_ptr.dis_ac = p_ptr.dis_ac + multiply_divide(inven(INVEN_WIELD+1).ac, p_ptr.abilities[(CLASS_DEFENDER * 10) + 2] * 3, 100)
		end
	end

	p_ptr.ac = p_ptr.ac + multiply_divide(p_ptr.ac, p_ptr.skill[5] * 3, 100)
	p_ptr.dis_ac = p_ptr.dis_ac + multiply_divide(p_ptr.dis_ac, p_ptr.skill[5] * 3, 100)

	-- Kobolds bonus to one-handed swords.
	if (p_ptr.prace == RACE_KOBOLD) then

		-- Check if we have two-handed weapons.
		if (not(get_object_flag4(inven(INVEN_WIELD), TR4_MUST2H)) and not(get_object_flag4(inven(INVEN_WIELD+1), TR4_MUST2H))) then

			-- If we wield swords, get a bonus.
			if (inven(INVEN_WIELD).itemskill == 12 or inven(INVEN_WIELD+1).itemskill == 12) then

				p_ptr.skill_bonus[13] = p_ptr.skill_bonus[13] + (p_ptr.skill_base[13] / 4)
			end
		end
	end

	-- Actually give the skills a value!
	calc_skills(2)

	-- If wearing hard or dragon armor, stealth is penalised.
	-- The penality is armor's weight * 3.
	if (inven(INVEN_BODY).tval == TV_HARD_ARMOR or inven(INVEN_BODY).tval == TV_DRAG_ARMOR) then

		local armorweight

		armorweight = (inven(INVEN_BODY).weight / 10) * 3
		if (armorweight > 100) then armorweight = 100 end

		p_ptr.skill[7] = multiply_divide(p_ptr.skill[7], armorweight, 100)
	end

	-- Kensai's Focus
   	if (p_ptr.abilities[(CLASS_KENSAI * 10) + 1] >= 1) then
		local totalwis
		totalwis = p_ptr.stat_cur[A_WIS+1] + p_ptr.stat_add[A_WIS+1] + p_ptr.stat_mut[A_WIS+1]
		if (p_ptr.wis_boost_dur > 0) then totalwis = totalwis + p_ptr.wis_boost end

		p_ptr.stat_add[A_DEX+1] = p_ptr.stat_add[A_DEX+1] + ((totalwis * 5 * (1 + (p_ptr.abilities[(CLASS_KENSAI * 10) + 1] / 2))) / 100)
		p_ptr.pspeed = p_ptr.pspeed + ((totalwis * 5 * (1 + (p_ptr.abilities[(CLASS_KENSAI * 10) + 1] / 2))) / 100)
		p_ptr.to_d = p_ptr.to_d + (totalwis * p_ptr.abilities[(CLASS_KENSAI * 10) + 1])
		p_ptr.dis_to_d = p_ptr.dis_to_d + (totalwis * p_ptr.abilities[(CLASS_KENSAI * 10) + 1])

   	end
	
	-- Calculate stats
	calc_stats(1)

	-- Priest blessings.
	if (p_ptr.events[29054] == 3) then

		local wispercent
		wispercent = 10 * p_ptr.abilities[(CLASS_PRIEST * 10) + 7]
		if (wispercent > 100) then wispercent = 100 end

		p_ptr.stat_ind[A_STR+1] = p_ptr.stat_ind[A_STR+1] + multiply_divide(p_ptr.stat_ind[A_WIS+1], wispercent, 100)
	end
	if (p_ptr.events[29054] == 4) then

		local wispercent
		wispercent = 10 * p_ptr.abilities[(CLASS_PRIEST * 10) + 8]
		if (wispercent > 100) then wispercent = 100 end

		p_ptr.stat_ind[A_DEX+1] = p_ptr.stat_ind[A_DEX+1] + multiply_divide(p_ptr.stat_ind[A_WIS+1], wispercent, 100)
	end

	-- Make sure we still have enough charisma for the song we currently sing.
	if (p_ptr.events[29042] == 1) then

		local cursong
		cursong = p_ptr.events[29041]
		if (p_ptr.stat_ind[A_CHR+1] < (music_song[cursong+1].cost - (p_ptr.skill_base[29] / 2))) then

			p_ptr.events[29041] = 0
			p_ptr.events[29042] = 0
		end
	end

	-- Apply Agility speed bonus here.
	p_ptr.pspeed = p_ptr.pspeed + (p_ptr.skill[6] / 5)

	-- Extract the current weight (in tenth pounds)
	j = total_weight

	-- Extract the "weight limit" (in tenth pounds)
	i = weight_limit()

	-- XXX XXX XXX Apply "encumbrance" from weight
	if (j > i/2) then p_ptr.pspeed = p_ptr.pspeed - ((j - (i/2)) / (i / 10)) end

	-- Searching slows the player down
	if (p_ptr.searching > 0) then p_ptr.pspeed = p_ptr.pspeed - 10 end

	-- Modifiers(to_h, to_d, to_a, etc...)

	-- Temporary shield
	if (p_ptr.shield > 0) then

                p_ptr.to_a = p_ptr.to_a + p_ptr.shield_power
                p_ptr.dis_to_a = p_ptr.dis_to_a + p_ptr.shield_power
	end

	-- Temporary "fast"
	if (p_ptr.fast > 0) then

		p_ptr.pspeed = p_ptr.pspeed + 10
	end

	-- Temporary "slow"
	if (p_ptr.slow > 0) then

		p_ptr.pspeed = p_ptr.pspeed - 10
	end

	-- Apply temporary "stun"
	if (p_ptr.stun > 50) then

		p_ptr.to_h = p_ptr.to_h - (p_ptr.to_h / 2)
		p_ptr.dis_to_h = p_ptr.dis_to_h - (p_ptr.dis_to_h / 2)
		p_ptr.to_d = p_ptr.to_d - (p_ptr.to_d / 2)
		p_ptr.dis_to_d = p_ptr.dis_to_d - (p_ptr.dis_to_d / 2)

	elseif (p_ptr.stun > 0) then

		p_ptr.to_h = p_ptr.to_h - (p_ptr.to_h / 4)
		p_ptr.dis_to_h = p_ptr.dis_to_h - (p_ptr.dis_to_h / 4)
		p_ptr.to_d = p_ptr.to_d - (p_ptr.to_d / 4)
		p_ptr.dis_to_d = p_ptr.dis_to_d - (p_ptr.dis_to_d / 4)
	end

	-- Temporary blessing
	if (p_ptr.blessed > 0) then

                p_ptr.ac = p_ptr.ac * 2
                p_ptr.dis_ac = p_ptr.dis_ac * 2
                p_ptr.to_h = p_ptr.to_h * 2
                p_ptr.dis_to_h = p_ptr.dis_to_h * 2
	end

	-- Temporary invisibility
        if (p_ptr.tim_invisible > 0) then
                               
                p_ptr.invis = p_ptr.invis + p_ptr.tim_inv_pow
	end

	-- Temporary ac boost! :)
        if (p_ptr.ac_boost_dur > 0) then

                p_ptr.to_a = p_ptr.to_a + p_ptr.ac_boost
                p_ptr.dis_to_a = p_ptr.dis_to_a + p_ptr.ac_boost
	end

	-- Fencer's Spirit sword skill!
        if (sword_has() and p_ptr.skill_base[13] >= 50) then

                p_ptr.to_h = p_ptr.to_h * 2
                p_ptr.dis_to_h = p_ptr.dis_to_h * 2
        end

	-- Temporary "Hero"
	if (p_ptr.hero > 0) then

		local herobonus

		if (p_ptr.events[29020] == 1) then
			herobonus = (p_ptr.abilities[(CLASS_KENSAI * 10) + 7] * 5)
		else
			herobonus = ((p_ptr.skill[11] + 1) * 5)
		end

		-- Limit the power of Heroism.
		if (herobonus > 500) then herobonus = 500 end

		p_ptr.to_h = p_ptr.to_h + (((p_ptr.to_h) * (herobonus)) / 100)
		p_ptr.dis_to_h = p_ptr.dis_to_h + (((p_ptr.dis_to_h) * (herobonus)) / 100)
		p_ptr.to_d = p_ptr.to_d + (((p_ptr.to_d) * (herobonus)) / 100)
		p_ptr.dis_to_d = p_ptr.dis_to_d + (((p_ptr.dis_to_d) * (herobonus)) / 100)
	end

	-- Not used at the moment, but keeping that old code.
	-- Temporary "Beserk"
	--if (p_ptr.shero > 0) then

	--	p_ptr.to_h = p_ptr.to_h + (p_ptr.to_h / 3) * (p_ptr.skill[11] + 1)
	--	p_ptr.dis_to_h = p_ptr.dis_to_h + (p_ptr.dis_to_h / 3) * (p_ptr.skill[11] + 1)
	--	p_ptr.to_d = p_ptr.to_d + (p_ptr.to_d / 3) * (p_ptr.skill[11] + 1)
	--	p_ptr.dis_to_d = p_ptr.dis_to_d + (p_ptr.dis_to_d / 3) * (p_ptr.skill[11] + 1)
	--	p_ptr.to_a = p_ptr.to_a - (p_ptr.to_a / 2)
	--	p_ptr.dis_to_a = p_ptr.dis_to_a - (p_ptr.dis_to_a / 2)
	--	p_ptr.to_a = p_ptr.to_a - 100
	--	p_ptr.dis_to_a = p_ptr.dis_to_a - 100
	--end

	-- Dual Wield adjustments.
	if (two_weapon_wield() and p_ptr.dualwield == 1) then

		local hitpenality
		local hitbonus
		local avgwgt

		avgwgt = ((inven(INVEN_WIELD).weight / 10) + (inven(INVEN_WIELD+1).weight / 10)) / 2
		if (p_ptr.skill[9] < ((avgwgt) * 4)) then

			-- With enough skill, daggers no longer cause penalities.
			if (not((inven(INVEN_WIELD).itemskill == 15 and inven(INVEN_WIELD+1).itemskill == 15) and (p_ptr.skill_base[16] >= 20))) then

				hitpenality = (((avgwgt) * 4) - p_ptr.skill[9]) * 2
				if (hitpenality >= 0) then

					if (hitpenality > 95) then hitpenality = 95 end
					p_ptr.to_h = p_ptr.to_h - ((p_ptr.to_h * hitpenality) / 100)
					p_ptr.dis_to_h = p_ptr.dis_to_h - ((p_ptr.dis_to_h * hitpenality) / 100)
				end
			end
		else
			hitbonus = (p_ptr.skill[9] - ((avgwgt) * 4))
			if (hitbonus > 0) then

				p_ptr.to_h = p_ptr.to_h + ((p_ptr.to_h * hitbonus) / 100)
				p_ptr.dis_to_h = p_ptr.dis_to_h + ((p_ptr.dis_to_h * hitbonus) / 100)
			end
		end
	end

        -- Bonus to_ac never go below 0
        if (p_ptr.to_a < 0) then p_ptr.to_a = 0 end
        if (p_ptr.dis_to_a < 0) then p_ptr.dis_to_a = 0 end

	-------------------------
	-- PASSIVE BONUS TO AC --
	-------------------------

	ac_bonus = ac_bonus + p_ptr.dis_to_a

	-- Battle Skill.
	if (p_ptr.abilities[(CLASS_WARRIOR * 10) + 4] >= 1) then

		ac_bonus = ac_bonus + (p_ptr.abilities[(CLASS_WARRIOR * 10) + 4] * 5)
        end

	p_ptr.ac = p_ptr.ac + multiply_divide(p_ptr.ac, ac_bonus, 100)
	p_ptr.dis_ac = p_ptr.dis_ac + multiply_divide(p_ptr.dis_ac, ac_bonus, 100)

        -- Base AC never go below 0.
        if (p_ptr.ac < 0) then p_ptr.ac = 0 end
        if (p_ptr.dis_ac < 0) then p_ptr.dis_ac = 0 end

	-------------------------
	-------------------------
	-------------------------

	-- Monk's Ki Resilience is applied after all bonus.
	if (p_ptr.abilities[(CLASS_MONK * 10) + 9] >= 1) then

		p_ptr.ac = p_ptr.ac + multiply_divide(p_ptr.stat_ind[A_WIS+1], p_ptr.abilities[(CLASS_MONK * 10) + 9] * 10, 100)
		p_ptr.dis_ac = p_ptr.dis_ac + multiply_divide(p_ptr.stat_ind[A_WIS+1], p_ptr.abilities[(CLASS_MONK * 10) + 9] * 10, 100)
	end

        -- If your shooting skill is 40+, get 1 extra shot!
        if (p_ptr.skill_base[3] >= 40) then

                p_ptr.num_fire = p_ptr.num_fire + 1
		p_ptr.num_fire2 = p_ptr.num_fire2 + 1
        end

	-- If you have throwing at 40+ without any ranged weapons, you get an extra shot.
        if (p_ptr.skill_base[4] >= 40 and not(inven(INVEN_WIELD).tval == TV_RANGED) and not(inven(INVEN_WIELD+1).tval == TV_RANGED)) then

                p_ptr.num_fire = p_ptr.num_fire + 1
        end

	-- If your Bows skill is 100+, you get a shot every 100 points.
	if (p_ptr.skill_base[20] >= 100) then

                if (inven(INVEN_WIELD).tval == TV_RANGED and (inven(INVEN_WIELD).itemskill + 1) == 20) then p_ptr.num_fire = p_ptr.num_fire + (p_ptr.skill_base[20] / 100) end
		if (inven(INVEN_WIELD+1).tval == TV_RANGED and (inven(INVEN_WIELD+1).itemskill + 1) == 20) then p_ptr.num_fire2 = p_ptr.num_fire2 + (p_ptr.skill_base[20] / 100) end
        end

	-- Extra blows and extra shots bonus.
	-- exblows and exshots are global variables affected in the calc_equipment function.
	extra_blows = extra_blows + exblows
	extra_shots = extra_shots + exshots

	-- Monsters can get some extra attacks.
	if (p_ptr.prace == RACE_MONSTER) then

		if (m_race(p_ptr.body_monster).attacks >= 2) then

			extra_blows = extra_blows + (m_race(p_ptr.body_monster).attacks - 1)
			extra_blows = extra_blows + (m_race(p_ptr.body_monster).attacksscale * (p_ptr.lev / m_race(p_ptr.body_monster).attacksscalefactor))
		end
	end

	-- Monsters can get some extra shots.
	if (p_ptr.prace == RACE_MONSTER) then

		if (m_race(p_ptr.body_monster).shots >= 2) then

			extra_shots = extra_shots + (m_race(p_ptr.body_monster).shots - 1)
			extra_shots = extra_shots + (m_race(p_ptr.body_monster).shotsscale * (p_ptr.lev / m_race(p_ptr.body_monster).shotsscalefactor))
		end
	end

	-- Obtain the "hold" value
        hold = max_carry() * 2

	-- Normal weapons

	-- Now include Sekira's "Weapon Katas", as well as a few fixes he did.
	if (inven(INVEN_WIELD).k_idx > 0) then

                local avg
		local i = 0
		local j = 0

                -- The average of strength and dexterity. Both of them
                -- can give you extra blows equally, so...
                avg = (p_ptr.stat_ind[A_STR+1] + p_ptr.stat_ind[A_DEX+1]) * 5

                -- Calculate the blows
                -- The average of str and dex divided by the weapon's weight
		if (inven(INVEN_WIELD).weight < 10) then
			j = 10
		else
			j = inven(INVEN_WIELD).weight
		end

		-- When dual wielding, the weapon's weight is treated two points higher.
		if (inven(INVEN_WIELD+1).k_idx > 0 and two_weapon_wield() and p_ptr.dualwield == 1) then

			j = j + 20
		end

		i = avg

		while (i > 0) do

                	--if (inven(INVEN_WIELD).weight < 10) then
			--	p_ptr.num_blow = p_ptr.num_blow + (avg / 10)
                	--else
			--	p_ptr.num_blow = p_ptr.num_blow + (avg / inven(INVEN_WIELD).weight)
			--end
			i = i - j
			if (i > 0) then

				p_ptr.num_blow = p_ptr.num_blow + 1
			end

			j = j + 10
		end

		-- Add in the "bonus blows"
		p_ptr.num_blow = p_ptr.num_blow + extra_blows

		-- Add in bonus from Martial Arts (Weapon Kata)
		--if (not(shield_has()) and not(inven(INVEN_WIELD).tval == TV_RANGED) and not(inven(INVEN_WIELD+1).tval == TV_RANGED)) then
		--	if (inven(INVEN_WIELD).weight < 100) then
		--		p_ptr.num_blow = p_ptr.num_blow + (p_ptr.skill[19] / 10)
		--	else
		--		p_ptr.num_blow = p_ptr.num_blow + (p_ptr.skill[19] * 10 / inven(INVEN_WIELD).weight)
		--	end
		--end

		-- Extra shots?
		--p_ptr.num_fire = p_ptr.num_fire + inven(INVEN_WIELD).extrashots
		-- Sekira's fix.
		p_ptr.num_fire = p_ptr.num_fire + extra_shots

		-- Dual Wielding reduces the number of blows if weapons are
		-- too heavy...
		-- It takes a skill of 3 times the weapon's weight to properly
		-- wield two of them.
		if (two_weapon_wield() and p_ptr.dualwield == 1) then

			-- With enough skill, daggers no longer cause penalities.
			if (not((inven(INVEN_WIELD).itemskill == 15 and inven(INVEN_WIELD+1).itemskill == 15) and (p_ptr.skill_base[16] >= 20))) then

				local blowpenality
				blowpenality = 0
				if (p_ptr.skill[9] < ((inven(INVEN_WIELD).weight / 10) * 3)) then

					blowpenality = (((inven(INVEN_WIELD).weight / 10) * 3) - p_ptr.skill[9]) / 5
					-- You never actually GAIN blows!
					if (blowpenality < 0) then blowpenality = 0 end
				end
				p_ptr.num_blow = p_ptr.num_blow - blowpenality
			end
		end

		-- Require at least one blow
		if (p_ptr.num_blow < 1) then p_ptr.num_blow = 1 end

        -- We're bare-handed...let's just calculate the blows using
        -- Martial Arts skill...
        elseif (unarmed()) then

                local avg
		local wisbonus = 0
		local i = 0
		local j = 0

		-- Monk's Strength through Spirit.
		if (p_ptr.abilities[(CLASS_MONK * 10) + 1] >= 1) then

			local wbonus

			wbonus = p_ptr.abilities[(CLASS_MONK * 10) + 1] * 10
			if (wbonus > 100) then wbonus = 100 end

			wisbonus = multiply_divide(p_ptr.stat_ind[A_WIS+1], wbonus, 100)
        	end

                -- The average of strength and dexterity. Both of them
                -- can give you extra blows equally, so...
                avg = ((p_ptr.stat_ind[A_STR+1] + wisbonus) + p_ptr.stat_ind[A_DEX+1]) / 2

                -- Calculate the blows
		if (p_ptr.prace == RACE_MONSTER) then
			i = avg
			j = 40

			while (i > 0) do

				i = i - j
				if (i > 0) then

					p_ptr.num_blow = p_ptr.num_blow + 1
					p_ptr.num_blow2 = p_ptr.num_blow2 + 1
				end

				j = j + 1
			end
		else
			if (not(inven(INVEN_BODY).tval == TV_HARD_ARMOR) and not(inven(INVEN_BODY).tval == TV_DRAG_ARMOR)) then

				i = avg
				j = 10

				while (i > 0) do

					i = i - j
					if (i > 0) then

						p_ptr.num_blow = p_ptr.num_blow + 1
						p_ptr.num_blow2 = p_ptr.num_blow2 + 1
					end

					j = j + 1
				end
			end
		end

		-- Add in the "bonus blows"
		p_ptr.num_blow = p_ptr.num_blow + extra_blows
		p_ptr.num_blow2 = p_ptr.num_blow2 + extra_blows

		--if (not(p_ptr.prace == RACE_MONSTER)) then
                	-- Add in some bonuses from the Martial Arts skill...
                --	p_ptr.num_blow = p_ptr.num_blow + (p_ptr.skill[19] / 10)
		--	p_ptr.num_blow2 = p_ptr.num_blow2 + (p_ptr.skill[19] / 10)

			-- Fighting skill can also help.
		--	p_ptr.num_blow = p_ptr.num_blow + (p_ptr.skill[1] / 20)
		--	p_ptr.num_blow2 = p_ptr.num_blow2 + (p_ptr.skill[1] / 20)
		--end

		-- Require at least one blow
		if (p_ptr.num_blow < 1) then p_ptr.num_blow = 1 end
		if (p_ptr.num_blow2 < 1) then p_ptr.num_blow2 = 1 end
        end

	-- The second weapon.
	if (inven(INVEN_WIELD+1).k_idx > 0) then

                local avg
		local i = 0
		local j = 0

		-- The average of strength and dexterity. Both of them
                -- can give you extra blows equally, so...
                avg = (p_ptr.stat_ind[A_STR+1] + p_ptr.stat_ind[A_DEX+1]) * 5

                -- Calculate the blows
		if (inven(INVEN_WIELD+1).weight < 10) then
			j = 10
		else
			j = inven(INVEN_WIELD+1).weight
		end

		-- When dual wielding, the weapon's weight is treated two points higher.
		if (inven(INVEN_WIELD).k_idx > 0 and two_weapon_wield() and p_ptr.dualwield == 1) then

			j = j + 20
		end

		i = avg

		while (i > 0) do

                	i = i - j
			if (i > 0) then

				p_ptr.num_blow2 = p_ptr.num_blow2 + 1
			end

			j = j + 10
		end

		-- Add in the "bonus blows"
		p_ptr.num_blow2 = p_ptr.num_blow2 + extra_blows

		-- Add in bonus from Martial Arts (Weapon Kata)
		--if (not(shield_has()) and not(inven(INVEN_WIELD).tval == TV_RANGED) and not(inven(INVEN_WIELD+1).tval == TV_RANGED)) then
		--	if (inven(INVEN_WIELD+1).weight < 100) then
		--		p_ptr.num_blow2 = p_ptr.num_blow2 + (p_ptr.skill[19] / 10)
		--	else
		--		p_ptr.num_blow2 = p_ptr.num_blow2 + (p_ptr.skill[19] * 10 / inven(INVEN_WIELD+1).weight)
		--	end
		--end

		-- Extra shots?
		--p_ptr.num_fire2 = p_ptr.num_fire2 + inven(INVEN_WIELD+1).extrashots
		-- Sekira's fix.
		p_ptr.num_fire2 = p_ptr.num_fire2 + extra_shots

		-- For the second weapon, unless we have a Dual Wield skill of 20+
		-- we only get half the number of blows.

		-- With enough skill, daggers no longer cause penalities.
		if (not((inven(INVEN_WIELD).itemskill == 15 and inven(INVEN_WIELD+1).itemskill == 15) and (p_ptr.skill_base[16] >= 20))) then

			if (p_ptr.skill_base[9] < 20) then p_ptr.num_blow2 = (p_ptr.num_blow2 / 2) end
		end

		-- Dual Wielding reduces the number of blows if weapons are
		-- too heavy...
		-- It takes a skill of 3 times the weapon's weight to properly
		-- wield two of them.
		if (two_weapon_wield() and p_ptr.dualwield == 1) then

			-- With enough skill, daggers no longer cause penalities.
			if (not((inven(INVEN_WIELD).itemskill == 15 and inven(INVEN_WIELD+1).itemskill == 15) and (p_ptr.skill_base[16] >= 20))) then

				local blowpenality
				blowpenality = 0
				if (p_ptr.skill[9] < ((inven(INVEN_WIELD+1).weight / 10) * 3)) then

					blowpenality = (((inven(INVEN_WIELD+1).weight / 10) * 3) - p_ptr.skill[9]) / 5
					-- You never actually GAIN blows!
					if (blowpenality < 0) then blowpenality = 0 end
				end
				p_ptr.num_blow2 = p_ptr.num_blow2 - blowpenality
			end
		end

		-- Require at least one blow
		if (p_ptr.num_blow2 < 1) then p_ptr.num_blow2 = 1 end
	end

        -- When fighting bare handed, use the martial arts skill to boost
	-- the damages. Note that claws are NOT considered martial arts!!

	-- Thanks to Sekira for patching this part! :)
	if (not(shield_has()) and not(inven(INVEN_WIELD).tval == TV_RANGED) and not(inven(INVEN_WIELD+1).tval == TV_RANGED)) then

		if (not(inven(INVEN_BODY).tval == TV_HARD_ARMOR) and not(inven(INVEN_BODY).tval == TV_DRAG_ARMOR)) then

			p_ptr.to_h = p_ptr.to_h + (p_ptr.skill[19] / 4)
			p_ptr.dis_to_h = p_ptr.dis_to_h + (p_ptr.skill[19] / 4)
			p_ptr.to_d = p_ptr.to_d + (p_ptr.skill[19] / 4)
			p_ptr.dis_to_d = p_ptr.dis_to_d + (p_ptr.skill[19] / 4)
		end
	end

	-- Perfect Union's speed bonus.
	if (p_ptr.abilities[(CLASS_MONK * 10) + 10] >= 1) then

		if (not(inven(INVEN_BODY).tval == TV_HARD_ARMOR) and not(inven(INVEN_BODY).tval == TV_DRAG_ARMOR)) then

			p_ptr.pspeed = p_ptr.pspeed + (p_ptr.abilities[(CLASS_MONK * 10) + 10])
		end
	end

        -- Shadow Stalker's Shadow Run!!
        if (p_ptr.abilities[(CLASS_SHADOW * 10) + 5] >= 1) then
 
                if (p_ptr.tim_invisible > 0) then

			local speedamount
                        speedamount = p_ptr.abilities[(CLASS_SHADOW * 10) + 5]
                        if (speedamount > p_ptr.tim_inv_pow) then speedamount = p_ptr.tim_inv_pow end
                        p_ptr.pspeed = p_ptr.pspeed + speedamount
                end
        end
        -- Shadow Stalker's One With Shadows!!
        if (p_ptr.abilities[(CLASS_SHADOW * 10) + 8] >= 1) then

                if (p_ptr.tim_invisible > 0) then

			local acbonus
                        acbonus = p_ptr.skill[7]
                        acbonus = acbonus + (acbonus * ((p_ptr.abilities[(CLASS_SHADOW * 10) + 8] * 10) / 100))
                        p_ptr.to_a = p_ptr.to_a + acbonus
                        p_ptr.dis_to_a = p_ptr.dis_to_a + acbonus
                end
        end

        -- Spell multiplier has a minimum of 1
        if (p_ptr.to_s == 0) then p_ptr.to_s = 1 end

	--(adelie) Combine the armor and speed checks to only run lua_update_stuff once if needed! 
	--(adelie) Note that this check must come after all possible modifiers to speed, to_a or ac!
	if ((not(p_ptr.pspeed == old_speed)) or (not(p_ptr.dis_to_a == old_dis_to_a)) or (not(p_ptr.dis_ac == old_dis_ac))) then
		lua_update_stuff()
	end

	-- Display Mighty Defense.
	if (p_ptr.events[29050] > 0) then

		if (p_ptr.events[29050] == 1) then
			c_put_str(TERM_L_GREEN, "MD: Melee  ", 15, 0)
		elseif (p_ptr.events[29050] == 2) then
			c_put_str(TERM_L_GREEN, "MD: Ranged ", 15, 0)
		elseif (p_ptr.events[29050] == 3) then
			c_put_str(TERM_L_GREEN, "MD: Magic  ", 15, 0)
		else
			c_put_str(TERM_L_GREEN, "MD: Summons", 15, 0)
		end
	else
		c_put_str(TERM_L_GREEN, "           ", 15, 0);
	end

end

-- Event handlers
add_event_handler("calc_hitpoints", calc_hitpoints)
add_event_handler("calc_mana", calc_mana)
add_event_handler("calc_bonuses", calc_bonuses)
add_event_handler("only_enchanted_weapons", only_enchanted_weapons)
add_event_handler("kensai_equip", kensai_equip)
