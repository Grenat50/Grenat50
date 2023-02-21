// Copyright (c) Mystic Project Dev Teams

#ifdef M5B00097
/* 
 * Return summon power rate bonus
 */
BUILDIN_FUNC(getSummonPowerBonusRate) {
	TBL_PC* sd;

	if (!script_rid2sd(sd))
		return SCRIPT_CMD_FAILURE;

	script_pushint(st,sd->special_state.summon_power_rate);

	return SCRIPT_CMD_SUCCESS;
}
#endif

#ifdef M5B00107
/* 
 * Return added Map Item rate (0.x%) of the player
 */
BUILDIN_FUNC(getMapItemRate) {
	TBL_PC* sd;

	if (!script_rid2sd(sd))
		return SCRIPT_CMD_FAILURE;

	script_pushint(st,sd->special_state.add_mapitem_rate);

	return SCRIPT_CMD_SUCCESS;
}

/* 
 * Return added Map Item quality (+x and x%) of the player
 */
BUILDIN_FUNC(getMapItemQuality) {
	TBL_PC* sd;

	if (!script_rid2sd(sd))
		return SCRIPT_CMD_FAILURE;

	script_pushint(st,sd->special_state.add_mapitem_quality);

	return SCRIPT_CMD_SUCCESS;
}
#endif

#ifdef M5G00017
/* Add Limit Point to player
 * addLP <value>;
 * @param value: LP amount
 * Return updated LP
 */
BUILDIN_FUNC(refreshlp) {
	TBL_PC* sd;

	if (!script_rid2sd(sd))
		return SCRIPT_CMD_FAILURE;

	clif_lp_change(sd);

	return SCRIPT_CMD_SUCCESS;
}
BUILDIN_FUNC(addLP) {
	int v = 0;
	TBL_PC* sd;

	if (!script_rid2sd(sd))
		return SCRIPT_CMD_FAILURE;
	v = script_getnum(st,2);

	if (!sd || v <= 0)
		return SCRIPT_CMD_FAILURE;

	if(sd->battle_status.lp + v > sd->battle_status.max_lp)
		v = sd->battle_status.max_lp - sd->battle_status.lp;

	sd->battle_status.lp += v;

	clif_lp_change(sd);

	script_pushint(st,sd->battle_status.lp);

	return SCRIPT_CMD_SUCCESS;
}

/* Substract Limit Point to player
 * subLP <value>;
 * @param value: LP amount
 * Return updated LP
 */
BUILDIN_FUNC(subLP) {
	int v = 0;
	TBL_PC* sd;

	if (!script_rid2sd(sd))
		return SCRIPT_CMD_FAILURE;
	v = script_getnum(st,2);

	if (!sd || v <= 0)
		return SCRIPT_CMD_FAILURE;

	sd->battle_status.lp -= v;

	if(sd->battle_status.lp < 0)
		sd->battle_status.lp = 0;

	clif_lp_change(sd);

	script_pushint(st,sd->battle_status.lp);

	return SCRIPT_CMD_SUCCESS;
}

/* Return actual Limit Point to player
 * getLP;
 */
BUILDIN_FUNC(getLP) {
	TBL_PC* sd;

	if (!script_rid2sd(sd))
		return SCRIPT_CMD_FAILURE;

	script_pushint(st,sd->battle_status.lp);

	return SCRIPT_CMD_SUCCESS;
}

/* Directly set given Limit Point to player
 * setLP <value>;
 * @param value: LP amount
 * Return updated LP
 */
BUILDIN_FUNC(setLP) {
	int v = 0;
	struct map_session_data *sd = NULL;

	if (!script_rid2sd(sd))
		return SCRIPT_CMD_FAILURE;
	v = script_getnum(st,2);

	if(v > sd->battle_status.max_lp)
		v = sd->battle_status.max_lp;

	if (v <= 0)
		return SCRIPT_CMD_SUCCESS;
	
	sd->battle_status.lp = v;

	script_pushint(st,sd->battle_status.lp);

	clif_lp_change(sd);
	return SCRIPT_CMD_SUCCESS;
}
#endif

#ifdef M5U00054
/**
 * setstatus(<effect type>,<type>,<new_value>{,<GID>}});
 **/
BUILDIN_FUNC(setstatus)
{
	int id, type, new_value;
	struct block_list* bl;

	if (!script_rid2bl(5,bl))
		return SCRIPT_CMD_FAILURE;

	struct status_change *sc = status_get_sc(bl);

	id = script_getnum(st, 2);
	type = script_getnum(st, 3);
	new_value = script_getnum(st, 4);

	if( id <= SC_NONE || id >= SC_MAX )
	{// invalid status type given
		ShowWarning("script.cpp:getstatus: Invalid status type given (%d).\n", id);
		return SCRIPT_CMD_SUCCESS;
	}

	if( sc->count == 0 || !sc->data[id] )
	{// no status is active
		script_pushint(st, 0);
		return SCRIPT_CMD_SUCCESS;
	}

	switch( type )
	{
		case 1:	 sc->data[id]->val1 = new_value;	break;
		case 2:  sc->data[id]->val2 = new_value;	break;
		case 3:  sc->data[id]->val3 = new_value;	break;
		case 4:  sc->data[id]->val4 = new_value;	break;
		case 5:
			{
				struct TimerData* timer = (struct TimerData*)get_timer(sc->data[id]->timer);
				if (timer)
					script_pushint(st, sc->data[id]->timer = (int)timer->tick + new_value);
			}
			break;
		case 6: sc->data[id]->timer = (int)gettick() + new_value; break;
		default: script_pushint(st, 0); break;
	}

	return SCRIPT_CMD_SUCCESS;
}
#endif

#ifdef M5U00040
static TIMER_FUNC(status_heal_hp_timer) {
	status_heal(map_id2bl(id), data, 0, 1);
	return 0;
}
static TIMER_FUNC(status_heal_sp_timer) {
	status_heal(map_id2bl(id), 0, data, 1);
	return 0;
}
/*==========================================
 * areastatus <map>,<x1>,<y1>,<x2>,<y2>,<status>,<val1>,<val2>,<val3>,<val4>,<duration>,{<type> 1 = Party, 0 = All // Grenat}
 *------------------------------------------*/
static int buildin_areastatus_sub2(struct block_list *bl, va_list ap)
{
	if (!bl)
		return 0;
	int status,val1,val2,val3,val4,duration;
	status = va_arg(ap, int);
	val1 = va_arg(ap, int);
	val2 = va_arg(ap, int);
	val3 = va_arg(ap, int);
	val4 = va_arg(ap, int);
	duration = va_arg(ap, int);
	if(status >= 9990) {	// Special case for Chronomancer Specialization [Grenat]
		sc_type types[16] = {SC_STONE,SC_FREEZE,SC_SLEEP,SC_CURSE,SC_STOP,SC_QUAGMIRE,SC_DECREASEAGI,SC_CLOSECONFINE,SC_CLOSECONFINE2,SC_BLADESTOP,SC_SLOW,SC_CHILL,SC_SLOWCAST,SC_SLOWDOWN,SC_ANKLE,SC_DEBUFF_WALK};
		for(int i = 0; i < ARRAYLENGTH(types); i++)
			status_change_end(bl, types[i], INVALID_TIMER);
		clif_specialeffect(bl, 1009, AREA);
#ifdef M5U00049
		status_change_start(bl, bl, SC_WALK_PENALTY, 10000, 10*(val1-9990), 0, 0, 0, 7000, 0);
#endif
		return 0;
	}
	status_change_start(bl, bl, (sc_type)status, 10000, val1, val2, val3, val4, duration, 0);
	return 0;
}
static int buildin_areastatus_sub(struct block_list *bl, va_list ap)
{
	int status,val1,val2,val3,val4,duration;
	int skiptarget = 0;
	int src_id = va_arg(ap, int);
	struct block_list* src = map_id2bl(src_id);
	if (!bl || !src)
		return 0;
	status = va_arg(ap, int);
	val1 = va_arg(ap, int);
	val2 = va_arg(ap, int);
	val3 = va_arg(ap, int);
	val4 = va_arg(ap, int);
	duration = va_arg(ap, int);
	if(bl->type == BL_PC) {
		struct map_session_data* sd = map_id2sd(src_id);
		struct map_session_data* tsd = map_id2sd(bl->id);
		if(tsd->status.party_id == sd->status.party_id || tsd->status.guild_id == sd->status.guild_id)
			skiptarget = 1;
	}
	if(status == SC_PROVOKE) {
		struct mob_data *md = map_id2md(bl->id);
		if(md) {
			clif_skill_nodamage(src, bl, SM_PROVOKE, 1, 1);
			md->state.provoke_flag = src->id;
			mob_target(md, src, 9);
		}
		
	}
	if(!skiptarget)
		status_change_start(bl, bl, (sc_type)status, 10000, val1, val2, val3, val4, duration, 0);
	return 0;
}
BUILDIN_FUNC(areastatus)
{
	int status,val1,val2,val3,val4,duration,m;
	const char *mapname;
	int x0,y0,x1,y1,target;

	mapname=script_getstr(st,2);
	x0=script_getnum(st,3);
	y0=script_getnum(st,4);
	x1=script_getnum(st,5);
	y1=script_getnum(st,6);
	status=script_getnum(st,7);
	val1=script_getnum(st,8);
	val2=script_getnum(st,9);
	val3=script_getnum(st,10);
	val4=script_getnum(st,11);
	duration=script_getnum(st,12);

	if( (m=map_mapname2mapid(mapname))< 0)
		return SCRIPT_CMD_FAILURE;

	if (script_hasdata(st, 13)) {
		struct map_session_data* sd = map_id2sd(st->rid);
		if (sd) {
			if(sd->status.party_id)
				party_foreachsamemap(buildin_areastatus_sub2, sd, x1-(sd->bl.x), status, val1, val2, val3, val4, duration);
			else {
				if(status >= 9990) {	// Special case for Chronomancer Specialization [Grenat]
					sc_type types[16] = {SC_STONE,SC_FREEZE,SC_SLEEP,SC_CURSE,SC_STOP,SC_QUAGMIRE,SC_DECREASEAGI,SC_CLOSECONFINE,SC_CLOSECONFINE2,SC_BLADESTOP,SC_SLOW,SC_CHILL,SC_SLOWCAST,SC_SLOWDOWN,SC_ANKLE,SC_DEBUFF_WALK};
					for(int i = 0; i < ARRAYLENGTH(types); i++)
						status_change_end(&sd->bl, types[i], INVALID_TIMER);
					clif_specialeffect(&sd->bl, 1009, AREA);
#ifdef M5U00049
					status_change_start(&sd->bl, &sd->bl, SC_WALK_PENALTY, 10000, 10*(val1-9990), 0, 0, 0, 7000, 0);
#endif
					return SCRIPT_CMD_SUCCESS;
				}
				status_change_start(&sd->bl, &sd->bl, (sc_type)status, 10000, val1, val2, val3, val4, duration, 0);
			}	
		}
	} else {
		if(map_getmapflag(m, MF_PVP) || map_getmapflag(m, MF_GVG))
			target = BL_PC|BL_MOB;
		else
			target = BL_MOB;
		map_foreachinallarea(buildin_areastatus_sub,m,x0,y0,x1,y1,target,st->rid,status,val1,val2,val3,val4,duration);
	}
	return SCRIPT_CMD_SUCCESS;
}
static int buildin_areabattledamage_sub(struct block_list *bl, va_list ap)
{
	if (!bl)
		return 0;
	int damage,ratio,element,modes,options,heal;
	int skiptarget = 0;
	int src_id = va_arg(ap, int);
	struct block_list* src = map_id2bl(src_id);
	if (!src || !bl)
		return 0;
	damage = va_arg(ap, int);
	ratio = va_arg(ap, int);
	element = va_arg(ap, int);
	modes = va_arg(ap, int);
	options = va_arg(ap, int);
	heal = va_arg(ap, int);

	struct map_session_data* sd = map_id2sd(src_id);
	if(sd && bl->type == BL_PC) {
		struct map_session_data* tsd = map_id2sd(bl->id);
		if(sd->status.party_id && tsd->status.party_id == sd->status.party_id)
			skiptarget = 1;
	}
	if(skiptarget)
		return 0;

	struct status_data *sstatus, ///< Attacker status data
		*tstatus; ///< Target status data
	sstatus = status_get_status_data(src);
	tstatus = status_get_status_data(bl);
	struct status_change *tsc = status_get_sc(bl);
	struct status_change *ssc = status_get_sc(src);

	int min_atk = calc_pc_min_atk(sstatus);
	int max_atk = calc_pc_max_atk(sstatus);
	int rand_atk = calc_pc_rand_atk(sstatus);

	if (modes > 0) {
		if (modes&DMG_ATK) // Damage/heals are based on self's average ATK
			damage += (min_atk + rand()%(rand_atk)) * ratio / 100;
		if (modes&DMG_ATK1) // Damage/heals are based on self's minimum ATK
			damage += min_atk * ratio / 100;
		if (modes&DMG_ATK2) // Damage/heals are based on self's maximum ATK
			damage += max_atk * ratio / 100;
		if (modes&DMG_MATK) // Damage/heals are based on self's average MATK
			damage += (sstatus->matk_min + rand()%(sstatus->matk_max - sstatus->matk_min > 0 ? sstatus->matk_max - sstatus->matk_min : 1)) * ratio / 100;
		if (modes&DMG_MATK1) // Damage/heals are based on self's minimum MATK
			damage += sstatus->matk_min * ratio / 100;
		if (modes&DMG_MATK2) // Damage/heals are based on self's maximum MATK
			damage += sstatus->matk_max * ratio / 100;
		if (modes&DMG_CSP) // Damage/heals are based on self's current SP
			damage += sstatus->sp * ratio / 100;
		if (modes&DMG_LSP) // Damage/heals are based on self's lost SP
			damage += (sstatus->max_sp - sstatus->sp) * ratio / 100;
		if (modes&DMG_CHP) // Damage/heals are based on self's current HP
			damage += sstatus->hp * ratio / 100;
		if (modes&DMG_LHP) // Damage/heals are based on self's lost HP
			damage += (sstatus->max_hp - sstatus->hp) * ratio / 100;

		if (modes&DMG_DEF) // Damage/heals are based on self's DEF
			damage += sstatus->def * damage / 100;
		if (modes&DMG_MDEF) // Damage/heals are based on self's MDEF
			damage += sstatus->mdef * damage / 100;
		if (modes&DMG_CRI) // Damage/heals are based on self's CRI
			damage += sstatus->cri * damage / 1000;
		if (modes&DMG_STR) // Damage/heals are based on self's STR
			damage += sstatus->str * damage / 100;
		if (modes&DMG_DEX) // Damage/heals are based on self's DEX
			damage += sstatus->dex * damage / 100;
		if (modes&DMG_INT) // Damage/heals are based on self's INT
			damage += sstatus->int_ * damage / 100;
		if (modes&DMG_VIT) // Damage/heals are based on self's VIT
			damage += sstatus->vit * damage / 100;
		if (modes&DMG_AGI) // Damage/heals are based on self's AGI
			damage += sstatus->agi * damage / 100;
		if (modes&DMG_LUK) // Damage/heals are based on self's LUK
			damage += sstatus->luk * damage / 100;

		if (modes&DMG_HPLEFT) // Damage/heals are based on self's current HP (the higher, the better)
			damage = damage * (100 + (sstatus->hp * 100 / (sstatus->max_hp > 0 ? sstatus->max_hp : 1))) / 100;
		if (modes&DMG_HPLOST) // Damage/heals are based on self's current HP (the lower, the better)
			damage = damage * (100 + (100 - sstatus->hp * 100 / (sstatus->max_hp > 0 ? sstatus->max_hp : 1))) / 100;
		if (modes&DMG_SPLEFT) // Damage/heals are based on self's current SP (the higher, the better)
			damage = damage * (100 + (sstatus->sp * 100 / (sstatus->max_sp > 0 ? sstatus->max_sp : 1))) / 100;
		if (modes&DMG_SPLOST) // Damage/heals are based on self's current SP (the lower, the better)
			damage = damage * (100 + (100 - sstatus->sp * 100 / (sstatus->max_sp > 0 ? sstatus->max_sp : 1))) / 100;
	}

	// Options
	if (options&DEF_DEF) // Damage/heals are reduced by target's DEF
		damage = damage * (100 - tstatus->def * (100 - ((modes&DMG_IDEF50) ? 50 : 0)) / 100) / 100;
	if (options&DEF_MDEF) // Damage/heals are reduced by target's MDEF
		damage = damage * (100 - tstatus->mdef * (100 - ((modes&DMG_IMDEF50) ? 50 : 0)) / 100) / 100;
	if (options&DEF_STR) // Damage/heals are reduced by target's STR
		damage = damage * 100 / (100 + tstatus->str);
	if (options&DEF_DEX) // Damage/heals are reduced by target's DEX
		damage = damage * 100 / (100 + tstatus->dex);
	if (options&DEF_INT) // Damage/heals are reduced by target's INT
		damage = damage * 100 / (100 + tstatus->int_);
	if (options&DEF_VIT) // Damage/heals are reduced by target's VIT
		damage = damage * 100 / (100 + tstatus->vit);
	if (options&DEF_AGI) // Damage/heals are reduced by target's AGI
		damage = damage * 100 / (100 + tstatus->agi);
	if (options&DEF_LUK) // Damage/heals are reduced by target's LUK
		damage = damage * 100 / (100 + tstatus->luk);
	if (options&DEF_RDEF) // Damage/heals are increased by target's DEF
		damage = damage * (100 + tstatus->def) / 100;
	if (options&DEF_RMDEF) // Damage/heals are increased by target's MDEF
		damage = damage * (100 + tstatus->mdef) / 100;
	if (options&DEF_RSTR) // Damage/heals are increased by target's STR
		damage = damage * (100 + tstatus->str / 2) / 100;
	if (options&DEF_RDEX) // Damage/heals are increased by target's DEX
		damage = damage * (100 + tstatus->dex / 2) / 100;
	if (options&DEF_RINT) // Damage/heals are increased by target's INT
		damage = damage * (100 + tstatus->int_ / 2) / 100;
	if (options&DEF_RVIT) // Damage/heals are increased by target's VIT
		damage = damage * (100 + tstatus->vit / 2) / 100;
	if (options&DEF_RAGI) // Damage/heals are increased by target's AGI
		damage = damage * (100 + tstatus->agi / 2) / 100;
	if (options&DEF_RLUK) // Damage/heals are increased by target's LUK
		damage = damage * (100 + tstatus->luk / 2) / 100;
	if (options&DEF_SPLEFT && bl->type == BL_PC) // Damage/heals are increased by target's current SP ratio
		damage = damage * (100 + (tstatus->sp * 100 / (tstatus->max_sp > 0 ? tstatus->max_sp : 1))) / 100;
	if (options&DEF_SPLOST && bl->type == BL_PC) // Damage/heals are increased by target's current HP (the lower, the better)
		damage = damage * (100 + (100 - tstatus->sp * 100 / (tstatus->max_sp > 0 ? tstatus->max_sp : 1))) / 100;
	if (options&DEF_HPLEFT) // Damage/heals are increased by target's current HP (the higher, the better)
		damage = damage * (100 + (tstatus->hp * 100 / (tstatus->max_hp > 0 ? tstatus->max_hp : 1))) / 100;
	if (options&DEF_HPLOST) // Damage/heals are increased by target's current HP (the lower, the better)
		damage = damage * (100 + (100 - tstatus->hp * 100 / (tstatus->max_hp > 0 ? tstatus->max_hp : 1))) / 100;

	// Each modifiers increase damage by 50%
	int modifier = 0;
	if (options&DEF_MOB && bl->type == BL_MOB)
		modifier += 1;
	if (options&DEF_BOSS && bl->type == BL_MOB && status_has_mode(tstatus,MD_STATUSIMMUNE))
		modifier += 1;
	if (options&DEF_MVP && bl->type == BL_MOB && status_has_mode(tstatus,MD_MVP))
		modifier += 1;
	if (options&DEF_FLEE)
		modifier += 1;

	// DEF_MODX1 add 50% for each modifiers and DEF_MODX2 add 100% for each modifiers, then are cumulative in order to add 150% per modifiers
	if (modifier > 0)
		damage += modifier * (1 + (options&DEF_MODX1 ? 1 : 0) + (options&DEF_MODX2 ? 2 : 0)) * damage / 2;

	if (element != ELE_NONE && (element = (element == ELE_WEAPON) ? ((sd && sd->bonus.arrow_ele) ? sd->bonus.arrow_ele : sstatus->rhw.ele) : element))
		damage = (int) battle_attr_fix(src, bl, (int64) damage, element, tstatus->def_ele, tstatus->ele_lv);

	if(heal) {
		if(!(options&DEF_SP))
			add_timer(gettick()+50,status_heal_hp_timer,bl->id,damage);
		else
			add_timer(gettick()+50,status_heal_sp_timer,bl->id,damage);
	} else {
		status_damage(src,bl,damage,0,0,0,0);
		clif_damage(bl, bl, gettick(), 500, 100, damage, 1, DMG_ENDURE, damage, true);
	}
  
  /*
	if (heal)
		modes += DMG_HEAL;

	extra_damage(src, bl, sstatus, tstatus, tsc, 0, ATK_DEF, damage, ratio, element, modes, options, ATK_DEF);
  */

	return 0;
}
BUILDIN_FUNC(areabattledamage)
{
	const char *mapname;
	int x0,y0,x1,y1,m,target;
	struct block_list* src;
	mapname=script_getstr(st,2);
	x0=script_getnum(st,3);
	y0=script_getnum(st,4);
	x1=script_getnum(st,5);
	y1=script_getnum(st,6);
	int damage = script_getnum(st,7);
	int ratio = script_getnum(st,8);
	int element = script_getnum(st,9);
	int modes = script_getnum(st,10);
	int options = script_getnum(st,11);
	int heal = 0;
	if(script_hasdata(st,12))
		heal = script_getnum(st,12);

	if(!script_rid2bl(13,src)) {
		script_pushint(st, 0);
		return SCRIPT_CMD_FAILURE;
	}

	if( (m=map_mapname2mapid(mapname))< 0)
		return SCRIPT_CMD_FAILURE;

	if(map_getmapflag(m, MF_PVP) || map_getmapflag(m, MF_GVG))
		target = BL_PC|BL_MOB;
	else
		target = BL_MOB;

	map_foreachinallarea(buildin_areabattledamage_sub,m,x0,y0,x1,y1,target,st->rid,damage,ratio,element,modes,options,heal);

	return SCRIPT_CMD_SUCCESS;
}
#endif

#ifdef M5U00055
/*==========================================
 * damage <rid>,<element>,<def>,<type>,<mode>,<amount>
 * element : ELE_NEUTRAL, etc...
 * def : 0 for none, 1 for DEF, 2 for MDEF, 3 for both
 * type : 0 for fixe value, 1 for percentage of maximum, 2 for pecentage of actual
 * target : 0 for HP, 1 for SP
 * mode : amount of damage
 *------------------------------------------*/
BUILDIN_FUNC(damage)
{
	int rid,element,def,type,mode,damage;

	rid = script_getnum(st,2);
	element=script_getnum(st,3);
	def=script_getnum(st,4);
	type=script_getnum(st,5);
	mode=script_getnum(st,6);
	damage=script_getnum(st,7);

	struct block_list *target = map_id2bl(rid);

	if(!target)
		return SCRIPT_CMD_FAILURE;

	struct status_data *status = status_get_status_data(target);

	if (element != ELE_NONE)
		damage = (int) battle_attr_fix(NULL, target, (int64) damage, element, status->def_ele, status->ele_lv);

	if (def&1)
		damage = damage * (100 - status->def) / 100 - status->def2;
	if (def&2)
		damage = damage * (100 - status->mdef) / 100 - status->mdef2;
	
	if (type == 1) {
        if (mode&1) {
            damage = damage * status->max_sp / 100;
        } else {
            damage = damage * status->max_hp / 100;
        }
    } else if (type == 2) {
        if (mode&1) {
            damage = damage * status->sp / 100;
        } else {
            damage = damage * status->hp / 100;
        }
    }

	status_zap(target, mode&1 ? 0 : damage, mode&1 ? damage : 0);

	return SCRIPT_CMD_SUCCESS;
}
#endif

#ifdef M5U00005
BUILDIN_FUNC(enchantui){
	struct map_session_data* sd;

	if( !script_charid2sd(2,sd) ){
		return SCRIPT_CMD_FAILURE;
	}
	if( !sd->state.refineui_open ){
		clif_enchantui_open(sd);
	}

	return SCRIPT_CMD_SUCCESS;
}
BUILDIN_FUNC(qualityui){
	struct map_session_data* sd;

	if( !script_charid2sd(2,sd) ){
		return SCRIPT_CMD_FAILURE;
	}
	if( !sd->state.refineui_open ){
		clif_enchantui_open(sd);
	}

	return SCRIPT_CMD_SUCCESS;
}
#endif

#ifdef M5U00002
BUILDIN_FUNC(unitremove){
	int id;
	struct block_list *bl = NULL;
	id = script_getnum(st,2);

	bl = map_id2bl(id);
	if (bl && bl->type == BL_MOB)
		unit_free(bl,CLR_OUTSIGHT);

	return SCRIPT_CMD_SUCCESS;
}

/// Attaches the current npc or the target npc to the mob unit
///
/// unitattach <mob unit id>{,"<npc name>"};
BUILDIN_FUNC(unitattach){
	struct block_list *bl;

	bl = map_id2bl(script_getnum(st,2));
	if( bl != NULL && bl->type == BL_MOB )
	{
		TBL_MOB* md = (TBL_MOB*)bl;
		TBL_NPC* nd = NULL;

		if( script_hasdata(st,3) )
			nd = npc_name2id(script_getstr(st, 3));
		else
		{
			struct block_list* npc_bl = map_id2bl(st->oid);
			if( npc_bl != NULL && npc_bl->type == BL_NPC )
				nd = (TBL_NPC*)npc_bl;
		}

		if( nd != NULL )
			md->nd = nd;
	}

	return SCRIPT_CMD_SUCCESS;
}

/// Makes the mob assist the target unit as a slave
///
/// mobassist <mob unit id>,"<player name>";
/// mobassist <mob unit id>,<target id>;
BUILDIN_FUNC(unitfollow)
{
	struct block_list* bl;

	// get mob
	bl = map_id2bl(script_getnum(st,2));
	if( bl != NULL && bl->type == BL_MOB )
	{
		TBL_MOB* md = (TBL_MOB*)bl;
		struct block_list* target_bl = NULL;
		struct script_data* data;

		// get target
		data = script_getdata(st, 3);
		get_val(st, data);
		if( data_isstring(data) )
		{
			TBL_PC* sd = map_nick2sd(script_getstr(st, 3),false);
			if( sd != NULL )
				target_bl = &sd->bl;
		}
		if( target_bl == NULL )
			target_bl = map_id2bl(conv_num(st, data));

		// set mob as slave
		if( target_bl != NULL )
		{
			struct unit_data* ud;

			md->master_id = target_bl->id;
			md->state.killer = 1;
			mob_convertslave(md);
			ud = unit_bl2ud(bl);
			if( ud != NULL )
			{
				if( ud->target != 0 )
					md->target_id = ud->target;
				else if( ud->skilltarget != 0 )
					md->target_id = ud->skilltarget;
				if( md->target_id != 0 )
					unit_walktobl(&md->bl, map_id2bl(md->target_id), 65025, 2);
			}
		}
	}

	return 0;
}

/// Add a visual effect on a specific GID
BUILDIN_FUNC(specialeffect3)
{
	int unit_id, type;
	enum send_target target;
	struct block_list* bl;

	unit_id = script_getnum(st,2);
	type = script_getnum(st,3);
	target = AREA;
	if(script_hasdata(st,4))
		target = SELF;

	bl = map_id2bl(unit_id);
	if( bl != NULL )
		clif_specialeffect(bl, type, target);

	return 0;
}

//Push away a specific GID
BUILDIN_FUNC(unitpush)
{
	uint8 dir;
	int cells, dx, dy;
	struct block_list* bl = map_id2bl(script_getnum(st,2));

	dir = script_getnum(st,3);
	cells     = script_getnum(st,4);

	if(dir>7)
	{
		ShowWarning("buildin_pushpc: Invalid direction %d specified.\n", dir);
		script_reportsrc(st);

		dir%= 8;  // trim spin-over
	}

	if(!cells)
	{// zero distance
		return 0;
	}
	else if(cells<0)
	{// pushing backwards
		dir = (dir+4)%8;  // turn around
		cells     = -cells;
	}

	dx = dirx[dir];
	dy = diry[dir];

	if( bl != NULL ) {
		unit_blown(bl, dx, dy, cells, (enum e_skill_blown)(BLOWN_IGNORE_NO_KNOCKBACK));
		clif_blown(bl);
	}
	return 0;
}

//Heal/attack a specific GID
//unitheal GID,hp,sp,{DMG type (visual effect)}
BUILDIN_FUNC(unitheal)
{
	struct block_list* bl = map_id2bl(script_getnum(st,2));
	int hp,sp;

	hp=script_getnum(st,3);
	sp=script_getnum(st,4);

	if ( bl != NULL ) {
		if (hp > 0)
			status_heal(bl, hp, sp, 1);
		else
			status_damage(NULL, bl, -hp, -sp, 0, 0, 0);
			clif_damage(bl,bl, gettick(), 0, 0, -hp, 0, DMG_NORMAL, 0, false);
	}
	return 0;
}

//Damage a specitif GID without visual effet of loosing HP
BUILDIN_FUNC(unitheal2)
{
	struct block_list* bl = map_id2bl(script_getnum(st,2));
	int hp,sp;

	hp=script_getnum(st,3);
	sp=script_getnum(st,4);
	if ( bl != NULL ) {
		if (hp > 0)
			status_heal(bl, hp, sp, 1);
		else
			status_damage(NULL, bl, -hp, -sp, 0, 0, 0);
	}
	return 0;
}

//Remove visual effect
BUILDIN_FUNC(refresh)
{
	TBL_PC* sd;
	if( !script_rid2sd(sd) )
		return SCRIPT_CMD_SUCCESS;
	clif_refresh(sd);
#ifdef M5U00060
	unit_refresh( &sd->bl, true );
#endif
	return 0;
}

#ifdef M5U00041
//permanent effect for bl
//fixeffect <unit_id>,<effect>,<number>; (up to 10)
BUILDIN_FUNC(fixeffect2)
{
	struct block_list* bl = map_id2bl(script_getnum(st,2));

	if( bl == NULL)
		return 0;

	enum send_target target = AREA;
	int id, effect, i = 0;
	effect = script_getnum(st,3);
	id = ( script_getnum(st,4) - 1 );

	do {
		effect = script_getnum(st,3+i);
		id = (script_getnum(st,4+i) - 1);

		if( id < 0 || id > 9 )
		return 0;
		if( effect < -1 || effect > 2000 )
		return 0;

		bl->fixeffect[id] = effect;
		clif_specialeffect(bl, bl->fixeffect[id], target);

		i = i +2;
	} while( script_hasdata(st,3+i) && script_hasdata(st,4+i) );

	return 0;
}
//fixeffect <unit_id>,<number>; (up to 10)
BUILDIN_FUNC(removeeffect)
{
	struct block_list* bl = map_id2bl(script_getnum(st,2));

	if( bl == NULL)
		return 0;
	else
		bl->fixeffect[script_getnum(st,3)] = 0;

	return 0;
}
#endif
BUILDIN_FUNC(getfixeffect)
{
	TBL_PC* sd;
	if( !script_rid2sd(sd) )
		return SCRIPT_CMD_SUCCESS;
	int id = ( script_getnum(st,2) );

	if( id < 0 || id > 9 )
		return 0;

	script_pushint(st, sd->fixeffect[id]);

	return 0;
}
BUILDIN_FUNC(getfixeffect2)
{
	struct block_list* bl;
	if( !script_rid2bl(3,bl) )
		return SCRIPT_CMD_SUCCESS;
	int id = ( script_getnum(st,2) );

	if( id < 0 || id > 9 )
		return 0;

	script_pushint(st, bl->fixeffect[id]);

	return 0;
}

/// Makes the mob assist the target unit as a slave
///
/// mobassist <mob unit id>,"<player name>";
/// mobassist <mob unit id>,<target id>;
BUILDIN_FUNC(unitassist)
{
	struct block_list* mob_bl;

	// get mob
	mob_bl = map_id2bl(script_getnum(st,2));
	if( mob_bl != NULL && mob_bl->type == BL_MOB )
	{
		TBL_MOB* md = (TBL_MOB*)mob_bl;
		struct block_list* target_bl = NULL;
		struct script_data* data;

		// get target
		data = script_getdata(st, 3);
		get_val(st, data);
		if( data_isstring(data) )
		{
			TBL_PC* sd = map_nick2sd(script_getstr(st, 3),false);
			if( sd != NULL )
				target_bl = &sd->bl;
		}
		if( target_bl == NULL )
			target_bl = map_id2bl(script_getnum(st, 3));

		// set mob as slave
		if( target_bl != NULL )
		{
			struct unit_data* ud;

			md->master_id = target_bl->id;
			md->state.killer = 1;
			mob_convertslave(md);
			ud = unit_bl2ud(mob_bl);
			if( ud != NULL )
			{
				if( ud->target != 0 )
					md->target_id = ud->target;
				else if( ud->skilltarget != 0 )
					md->target_id = ud->skilltarget;
				if( md->target_id != 0 )
					unit_walktobl(&md->bl, map_id2bl(md->target_id), 65025, 2);
			}
		}
	}

	return 0;
}

/// Check if a GID is alive or dead
///
/// isdead(<GID>);
BUILDIN_FUNC(isdead)
{
	struct block_list *bl = map_id2bl(script_getnum(st,2));
	if(!bl) {
		script_pushint(st,0);
		return SCRIPT_CMD_SUCCESS;
	}

	switch(bl->type) {
		case BL_PC: script_pushint(st, (((TBL_PC*)bl)->state.dead_sit)); break;
		case BL_MOB: script_pushint(st, (((TBL_MOB*)bl)->spawn_timer != INVALID_TIMER)); break;
		case BL_HOM: script_pushint(st, ((((TBL_HOM*)bl)->homunculus.hp == 0))); break;
		default:
			ShowError("buildin_isdead: BL type unknown.\n");
			return SCRIPT_CMD_FAILURE;
	}

	return SCRIPT_CMD_SUCCESS;
}

// Forces a PJ walk to a defined position
// forcepcwalkto x,y,{,<char_id>};
BUILDIN_FUNC(forcepcwalkto)
{
	TBL_PC *sd;
	int x=0,y=0;

	x=script_getnum(st,2);
	y=script_getnum(st,3);

	if (!script_charid2sd(4,sd))
		return SCRIPT_CMD_FAILURE;

	if(sd->status.hp == 0)
		return SCRIPT_CMD_FAILURE;
	else
		unit_walktoxy(&sd->bl,x,y,0);

	return SCRIPT_CMD_SUCCESS;
}
#endif

#ifdef M5U00018
/**
 * Forces the summoned pet to return.
 * returnpet;
 */
BUILDIN_FUNC(returnpet)
{
	struct block_list* bl;
	TBL_PET* pd;

	if (script_hasdata(st, 2))
		bl = map_id2bl(script_getnum(st, 2));
	else
		bl = map_id2bl(st->rid);
	if(!bl)
		return SCRIPT_CMD_FAILURE;

	TBL_PC* sd = map_id2sd(bl->id);
	if(sd->pd == NULL)
		return SCRIPT_CMD_SUCCESS;

	pd = sd->pd;

	pet_lootitem_drop(pd,sd);

	int i = pet_egg_search( sd, pd->pet.pet_id );

	if( i == -1 ){
		return false;
 	}

	sd->inventory.u.items_inventory[i].attribute = 0;
	sd->inventory.dirty = true;
	pd->pet.incubate = 1;
	clif_inventorylist(sd);

#if PACKETVER >= 20180704
	clif_send_petdata(sd, pd, 6, 0);
#endif
	unit_free(&pd->bl,CLR_OUTSIGHT);

	if (sd->inventory.u.items_inventory[i].equip & equip_bitmask[EQI_SHADOW_ARMOR])
		pc_unequipitem(sd,pc_checkequip(sd,equip_bitmask[EQI_SHADOW_ARMOR]),2);

	status_change_end(&sd->bl, SC_PET, INVALID_TIMER);

	status_calc_pc(sd, SCO_FORCE);
	sd->status.pet_id = 0;

	return SCRIPT_CMD_SUCCESS;
}

/**
 * Summon a specific pet based on its class (if existed)
 * summonpet (<mob id>);
 */
BUILDIN_FUNC(summonpet)
{
	struct map_session_data *sd;
	int pet_id = script_getnum(st,2);

	if(!script_rid2sd(sd))
		return SCRIPT_CMD_FAILURE;

	intif_request_petdata(sd->status.account_id, sd->status.char_id, pet_id);

	sc_start( &sd->bl, &sd->bl, SC_PET, 100, 1, INFINITE_TICK );

	return SCRIPT_CMD_SUCCESS;

}

/**
 * Make a pet drop his loots or clear all bonuses.
 * petclear;
 */
BUILDIN_FUNC(petclear)
{
	struct map_session_data *sd;

	if(!script_rid2sd(sd))
		return SCRIPT_CMD_FAILURE;

	pet_clear_support_bonuses(sd);

	return SCRIPT_CMD_SUCCESS;
}

/**
 * Transfer completely from a pet to another. (item/name/random opt included)
 * changepet (<target mob_id>);
 */
BUILDIN_FUNC(changepet)
{
	TBL_PC* sd;
	TBL_PET* pd;

	if(!script_rid2sd(sd) || sd->pd == NULL)
		return SCRIPT_CMD_FAILURE;

	pd = sd->pd;

	if (sd->pd->pet.equip) {
		clif_displaymessage(sd->fd, "Please remove the pet accessory first.");
		return SCRIPT_CMD_FAILURE;
	}
	
	int pet_id = script_getnum(st,2);
	int pet_intimacy = sd->pd->pet.intimate;

	std::shared_ptr<s_pet_db> new_data = pet_db.find(pet_id);

	if( new_data == nullptr ){
		return SCRIPT_CMD_FAILURE;
	}

	int idx = pet_egg_search(sd, sd->pd->pet.pet_id);

	if( idx == -1 ){
		clif_pet_evolution_result(sd, e_pet_evolution_result::FAIL_NOTEXIST_CALLPET);
		return SCRIPT_CMD_FAILURE;
	}
#ifdef M5U00060
	sd->petnocalc = 1;
#endif
	// Taking the random opt from the old pet
	int pet_random_opt1_id = sd->inventory.u.items_inventory[idx].option[0].id;
	int pet_random_opt1_val = sd->inventory.u.items_inventory[idx].option[0].value;
	int pet_random_opt1_param = sd->inventory.u.items_inventory[idx].option[0].param;
	int pet_random_opt2_id = sd->inventory.u.items_inventory[idx].option[1].id;
	int pet_random_opt2_val = sd->inventory.u.items_inventory[idx].option[1].value;
	int pet_random_opt2_param = sd->inventory.u.items_inventory[idx].option[1].param;
	int pet_random_opt3_id = sd->inventory.u.items_inventory[idx].option[2].id;
	int pet_random_opt3_val = sd->inventory.u.items_inventory[idx].option[2].value;
	int pet_random_opt3_param = sd->inventory.u.items_inventory[idx].option[2].param;
	int pet_random_opt4_id = sd->inventory.u.items_inventory[idx].option[3].id;
	int pet_random_opt4_val = sd->inventory.u.items_inventory[idx].option[3].value;
	int pet_random_opt4_param = sd->inventory.u.items_inventory[idx].option[3].param;
	int pet_random_opt5_id = sd->inventory.u.items_inventory[idx].option[4].id;
	int pet_random_opt5_val = sd->inventory.u.items_inventory[idx].option[4].value;
	int pet_random_opt5_param = sd->inventory.u.items_inventory[idx].option[4].param;

	// Virtually delete the old egg
	log_pick_pc(sd, LOG_TYPE_OTHER, -1, &sd->inventory.u.items_inventory[idx]);
	clif_delitem(sd, idx, 1, 0);

	// Change the old egg to the new one
	sd->inventory.u.items_inventory[idx].nameid = new_data->EggID;
	sd->inventory_data[idx] = itemdb_search(new_data->EggID);

	// Virtually add it to the inventory
	log_pick_pc(sd, LOG_TYPE_OTHER, 1, &sd->inventory.u.items_inventory[idx]);
	clif_additem(sd, idx, 1, 0);

	// Adding the random opt from the old pet to the new one
	sd->inventory.u.items_inventory[idx].option[0].id = pet_random_opt1_id;
	sd->inventory.u.items_inventory[idx].option[0].value = pet_random_opt1_val;
	sd->inventory.u.items_inventory[idx].option[0].param = pet_random_opt1_param;
	sd->inventory.u.items_inventory[idx].option[1].id = pet_random_opt2_id;
	sd->inventory.u.items_inventory[idx].option[1].value = pet_random_opt2_val;
	sd->inventory.u.items_inventory[idx].option[1].param = pet_random_opt2_param;
	sd->inventory.u.items_inventory[idx].option[2].id = pet_random_opt3_id;
	sd->inventory.u.items_inventory[idx].option[2].value = pet_random_opt3_val;
	sd->inventory.u.items_inventory[idx].option[2].param = pet_random_opt3_param;
	sd->inventory.u.items_inventory[idx].option[3].id = pet_random_opt4_id;
	sd->inventory.u.items_inventory[idx].option[3].value = pet_random_opt4_val;
	sd->inventory.u.items_inventory[idx].option[3].param = pet_random_opt4_param;
	sd->inventory.u.items_inventory[idx].option[4].id = pet_random_opt5_id;
	sd->inventory.u.items_inventory[idx].option[4].value = pet_random_opt5_val;
	sd->inventory.u.items_inventory[idx].option[4].param = pet_random_opt5_param;

	// Remove the old pet from sight
	unit_remove_map(&sd->pd->bl, CLR_OUTSIGHT);

	sd->pd->pet.class_ = pet_id;
	sd->pd->pet.egg_id = new_data->EggID;
	pet_set_intimate(sd->pd, pet_intimacy);

	if( !sd->pd->pet.rename_flag ){
		std::shared_ptr<s_mob_db> mdb = mob_db.find( pet_id );

		safestrncpy(sd->pd->pet.name, mdb->jname.c_str(), NAME_LENGTH);
	}
	status_set_viewdata(&sd->pd->bl, pet_id);

	// Save the pet and inventory data
	intif_save_petdata(sd->status.account_id, &sd->pd->pet);
	if (save_settings&CHARSAVE_PET)
		chrif_save(sd, CSAVE_INVENTORY);

	// Spawn it
	if (map_addblock(&sd->pd->bl))
		return SCRIPT_CMD_FAILURE;
	clif_spawn(&sd->pd->bl);
	clif_send_petdata(sd, sd->pd, 0, 0);
	clif_send_petdata(sd, sd->pd, 5, battle_config.pet_hair_style);
	clif_pet_equip_area(sd->pd);
	clif_send_petstatus(sd);
	clif_emotion(&sd->bl, ET_BEST);
	clif_specialeffect(&sd->pd->bl, EF_HO_UP, AREA);
	clif_inventorylist(sd);

#ifdef M5U00060
	sd->petnocalc = 0;
#endif
	return SCRIPT_CMD_SUCCESS;
}

/**
* Adds a random option on a random option slot on an equipped item and overwrites
* existing random option in the process.
* setrandomoption(<equipment slot>,<index>,<id>,<value>,<param>{,<char id>});
* Mystic 5 Modification - No Recalc Stats !
*/
BUILDIN_FUNC(setpetrandomoption) {
	struct map_session_data *sd;
	std::shared_ptr<s_random_opt_data> opt;
	int pos, index, id, value, param, ep;
	int i = -1;
	if (!script_charid2sd(8, sd))
		return SCRIPT_CMD_FAILURE;
	pos = script_getnum(st, 2);
	index = script_getnum(st, 3);
	id = script_getnum(st, 4);
	value = script_getnum(st, 5);
	param = script_getnum(st, 6);

	if ((opt = random_option_db.find((short)id)) == NULL) {
		ShowError("buildin_setrandomoption: Random option ID %d does not exists.\n", id);
		script_pushint(st, 0);
		return SCRIPT_CMD_FAILURE;
	}
	if (index < 0 || index >= MAX_ITEM_RDM_OPT) {
		ShowError("buildin_setrandomoption: Invalid random option index %d.\n", index);
		script_pushint(st, 0);
		return SCRIPT_CMD_FAILURE;
	}
	if (equip_index_check(pos))
		i = pc_checkequip(sd, equip_bitmask[pos]);
	if (i >= 0) {
		ep = sd->inventory.u.items_inventory[i].equip;
//		log_pick_pc(sd, LOG_TYPE_SCRIPT, -1, &sd->inventory.u.items_inventory[i]);
		sd->inventory.u.items_inventory[i].option[index].id = id;
		sd->inventory.u.items_inventory[i].option[index].value = value;
		sd->inventory.u.items_inventory[i].option[index].param = param;
//		clif_delitem(sd, i, 1, 0);
//		log_pick_pc(sd, LOG_TYPE_SCRIPT, -1, &sd->inventory.u.items_inventory[i]);
//		clif_additem(sd, i, 1, 0);
		clif_inventorylist(sd);
		if(script_hasdata(st, 7))
			status_calc_pc(sd, SCO_FORCE);
		script_pushint(st, 1);
		return SCRIPT_CMD_SUCCESS;
	}

	ShowError("buildin_setrandomoption: No item equipped at pos %d (CID=%d/AID=%d).\n", pos, sd->status.char_id, sd->status.account_id);
	script_pushint(st, 0);
	return SCRIPT_CMD_FAILURE;
}
#endif

#ifdef M5U00019
/**************************************
/ Adding the script command: rankpvp() [Odesseiron]
/ => Return the PVP rank
**********************************************************/
BUILDIN_FUNC(rankpvp)
{
	TBL_PC* sd;
	int x=0,y=0;
	if(script_hasdata(st,2))
		sd = map_id2sd(script_getnum(st,2));
	else {
		if( !script_rid2sd(sd) )
			return SCRIPT_CMD_SUCCESS;
	}
	if(!sd)
		return 0;

	script_pushint(st, sd->pvp_rank);
	return 0;
}

#ifdef M5U00020
BUILDIN_FUNC(updatequest)
{
	TBL_PC *sd;
	int mob_id = script_getnum(st,2);
	int no_filter = 1;
	if(script_hasdata(st,3))
		no_filter = script_getnum(st,3);

	if( !script_rid2sd(sd) )
		return SCRIPT_CMD_FAILURE;

	if(!mobdb_checkid(mob_id))
		return SCRIPT_CMD_FAILURE;
	
	quest_update_objective(sd, (TBL_MOB*)mob_id, mob_id);
	return SCRIPT_CMD_SUCCESS;
}
#endif

#ifdef M5U00021
/**********************************************************
/ Additing the script command: monster2 [Grenat]
/ => Allows to create a monster with specified stats.
/ => Example: monster2 "map name",<x>,<y>,"Monster name",<monster id>,<amount>,<Time Out>,{"event label",<size>,<ai>,<type>
/ => hp,atk1,atk2,def,mdef,str,agi,vit,int,dex,luk,vitesse,aspd, <matk1>,<matk2>,<hit>,<flee>,<crit>,<pflee>,<amotion>,<dmotion>,<level>,<baseExp>,<jobExp>,<zeny>,<s_flag elite>};
/ => Return GID of the last monster.
**********************************************************/
static TIMER_FUNC(monster2_timer){
	struct mob_data *md = map_id2md(id);
	if(md)
		md->stance = 0;
	return 0;
}
BUILDIN_FUNC(monster2)
{
	struct block_list bl;
	struct mob_data *md;
	t_tick tick = gettick();
	int _class, i, amount, timeout;
	const char *str,*event = "";
	unsigned int size = SZ_SMALL;
	enum mob_ai ai = AI_ATTACK;
	bl.m = map_mapname2mapid(script_getstr(st,2));
	int x = script_getnum(st,3);
	int y = script_getnum(st,4);
	str = script_getstr(st,5);
	_class = script_getnum(st,6);
	amount = script_getnum(st,7);
	timeout = script_getnum(st,8);

	check_event(st, event);
	if (script_hasdata(st, 9)) {
		event = script_getstr(st, 9);
		check_event(st, event);
	}

	if (script_hasdata(st, 10)) {
		size = script_getnum(st, 10);
		if (size > SZ_BIG) {
			ShowWarning("buildin_monster: Attempted to spawn non-existing size %d for monster class %d\n", size, _class);
			return SCRIPT_CMD_FAILURE;
		}
	}

	if (script_hasdata(st, 11)) {
		ai = static_cast<enum mob_ai>(script_getnum(st, 11));
		if (ai >= AI_MAX) {
			ShowWarning("buildin_monster: Attempted to spawn non-existing ai %d for monster class %d\n", ai, _class);
			return SCRIPT_CMD_FAILURE;
		}
	}

	if (_class >= 0 && !mobdb_checkid(_class)) {
		ShowWarning("buildin_monster: Attempted to spawn non-existing monster class %d\n", _class);
		return SCRIPT_CMD_FAILURE;
	}

	for(i = 0; i < amount; i++) {

		md = mob_once_spawn_sub(&bl, bl.m, x, y, str, _class, event, size, ai);

		if (md) {
			if(timeout)
				md->deletetimer = add_timer(tick+timeout*1000,mob_timer_delete,md->bl.id,0);

			if (script_hasdata(st, 12)) {
				if(script_getnum(st, 12)) {
					struct map_session_data* sd = map_id2sd(st->rid);
					if( !script_rid2sd(sd) )
						return SCRIPT_CMD_SUCCESS;
					md->master_id = sd->bl.id;
					md->special_state.ai = ai;
					md->stance = SUMM_FREE;
					if(timeout <= 12 && !(md->status.mode&MD_AGGRESSIVE))
						md->status.mode = static_cast<enum e_mode>(md->status.mode|MD_AGGRESSIVE);
					add_timer(gettick()+600, monster2_timer, md->bl.id, 0);
				}
			}

			md->special_state.size = size;
#ifdef M5U00004
			md->norecalc = 1;
#endif
#ifdef M5U00004
			if(script_hasdata(st,38))
				md->s_flag = static_cast<enum special_flag>(script_getnum(st, 38));
#endif
			mob_spawn(md);
			clif_specialeffect(&md->bl, EF_ENTRY2, AREA);

			if (!md->base_status) {
				md->base_status = (struct status_data*)aCalloc(1, sizeof(struct status_data));
				memcpy(md->base_status, &md->db->status, sizeof(struct status_data));
			}

			if(script_hasdata(st,34)) { md->level = script_getnum(st,34); clif_name_area(&md->bl); }	//Level
			if(script_hasdata(st,16)) md->base_status->def = script_getnum(st,16);				//Def
			if(script_hasdata(st,17)) md->base_status->mdef = script_getnum(st,17);				//Mdef
			if(script_hasdata(st,18)) md->base_status->str = script_getnum(st,18);				//Str
			if(script_hasdata(st,19)) md->base_status->agi = script_getnum(st,19);				//Agi
			if(script_hasdata(st,20)) md->base_status->vit = script_getnum(st,20);				//Vit
			if(script_hasdata(st,21)) md->base_status->int_ = script_getnum(st,21);				//Int
			if(script_hasdata(st,22)) md->base_status->dex = script_getnum(st,22); 				//Dex
			if(script_hasdata(st,23)) md->base_status->luk = script_getnum(st,23); 				//Luk

			// We recalc here because we are touching stats that changes the course of the story !!
			status_calc_misc(&md->bl, md->base_status, md->level);
			status_calc_mob(md,SCO_NONE);

			md->base_status->def2 = md->base_status->vit;
			md->base_status->mdef2 = md->base_status->int_;
			if(script_hasdata(st,14)) md->base_status->rhw.atk = script_getnum(st,14);			//Atk1 (min)
			if(script_hasdata(st,15)) md->base_status->rhw.atk2 = script_getnum(st,15);			//Atk2 (max)
			if(script_hasdata(st,30)) md->status.speed = script_getnum(st,30); 					//Vitesse deplacement
			if(script_hasdata(st,30)) md->base_status->speed = script_getnum(st,30); 			//Vitesse deplacement
			if(script_hasdata(st,31)) md->status.adelay = script_getnum(st,31); 				//ASPD
			if(script_hasdata(st,31)) md->base_status->adelay = script_getnum(st,31); 			//ASPD
			if(script_hasdata(st,24)) md->base_status->matk_min = script_getnum(st,24);			//MATK1
			if(script_hasdata(st,25)) md->base_status->matk_max = script_getnum(st,25);			//MATK2
			if(script_hasdata(st,26)) md->status.hit = script_getnum(st,26);					//HIT
			if(script_hasdata(st,27)) md->status.flee = script_getnum(st,27);					//FLEE
			if(script_hasdata(st,28)) md->status.cri = script_getnum(st,28);					//CRIT
			if(script_hasdata(st,29)) md->status.flee2 = script_getnum(st,29);					//PFLEE
			if(script_hasdata(st,32)) md->status.amotion = script_getnum(st,32);				//aMotion
			if(script_hasdata(st,28)) md->base_status->hit = script_getnum(st,26);				//HIT
			if(script_hasdata(st,29)) md->base_status->flee = script_getnum(st,27);				//FLEE
			if(script_hasdata(st,30)) md->base_status->cri = script_getnum(st,28);				//CRIT
			if(script_hasdata(st,31)) md->base_status->flee2 = script_getnum(st,29);			//PFLEE
			if(script_hasdata(st,33)) md->base_status->dmotion = script_getnum(st,33);			//dMotion
#ifdef M5U00004
			if(script_hasdata(st,35)) md->base_exp = script_getnum(st,35);						//BaseExp
			if(script_hasdata(st,36)) md->job_exp = script_getnum(st,36);						//JobExp
#endif
#ifdef M5G00013
			if(script_hasdata(st,37)) md->zeny = script_getnum(st,37);							//Zeny
#endif
			status_calc_mob(md,SCO_NONE);
			if(script_hasdata(st,13)) {
               			md->base_status->hp = md->base_status->max_hp = script_getnum(st,13);
				status_set_hp(&md->bl, script_getnum(st,13), 0);
				status_set_maxhp(&md->bl, script_getnum(st,13), 0);
                		clif_name_area(&md->bl);
			}

			script_pushint(st, md->bl.id);
		}
	}

	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 * GetMapMobs
	returns mob counts on a set map:
	e.g. GetMapMobs("prontera")
	use "this" - for player's map
 *------------------------------------------*/
BUILDIN_FUNC(getmapmobs) {
	const char *str=NULL;
	int m=-1,bx,by;
	int count=0;
	struct block_list *bl;

	str=script_getstr(st,2);

	TBL_PC* sd;
	if( !script_rid2sd(sd) )
		return SCRIPT_CMD_SUCCESS;

	if(strcmp(str,"this")==0){
		if(sd)
			m=sd->bl.m;
		else{
			script_pushint(st,-1);
			return 0;
		}
	}else
		m=map_mapname2mapid(str);

	if(m < 0){
		script_pushint(st,-1);
		return 0;
	}

	for(by=0;by<=(map[m].ys-1)/BLOCK_SIZE;by++)
		for(bx=0;bx<=(map[m].xs-1)/BLOCK_SIZE;bx++)
			for( bl = map[m].block_mob[bx+by*map[m].bxs] ; bl != NULL ; bl = bl->next )
				if(bl->x>=0 && bl->x<=map[m].xs-1 && bl->y>=0 && bl->y<=map[m].ys-1)
					count++;

	script_pushint(st,count);
	return SCRIPT_CMD_SUCCESS;
}

/*************************************************************
Ajout de commande script: healsummon x,y; ou healsummon x;
=> x: Nombre de vie soigné. Si x est négatif, healsummon soigne de x%.
=> y: ID du summon à heal. (0 ou rien => Tout les summons du joueur)
*************************************************************/
BUILDIN_FUNC(healsummon) {
	struct block_list* bl;
	int m=-1,bx,by;
	int hp = script_getnum(st,2);
	struct status_data *tstatus;
	int id = script_hasdata(st,3) ? script_getnum(st,3) : 0;
	if(hp < -100 || hp == 0) { script_pushint(st, -1); return 0; }

	TBL_PC* sd;
	if( !script_rid2sd(sd) )
		return SCRIPT_CMD_SUCCESS;

	for(by=0;by<=(map[m].ys-1)/BLOCK_SIZE;by++)
		for(bx=0;bx<=(map[m].xs-1)/BLOCK_SIZE;bx++)
			for( bl = map[m].block[bx+by*map[m].bxs] ; bl != NULL ; bl = bl->next )
				if(bl->x>=0 && bl->x<=map[m].xs-1 && bl->y>=0 && bl->y<=map[m].ys-1)
					if(bl->type & BL_MOB) { // Mobs uniquement
						TBL_MOB* md = (TBL_MOB*)bl;
						if(id == md->mob_id || id == 0) // ID du mob demandé ou tout les ID.
							if(md->master_id == sd->bl.id) // Summon du joueur
								{
									if(hp > 0) status_heal(bl, hp, 0, 1); // Soins en valeur
									else if(hp < 0) { // Soins en pourcentage
										tstatus = status_get_status_data(bl);
										status_heal(bl,(tstatus->hp*abs(hp))/tstatus->max_hp, 0, 1); 
									}
								}
					}
	script_pushint(st, 1);
	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 * Get NPC info: getnpcinfo <type>{,<char_id>} [Grenat]
 * type: NPC_X: X, NPC_Y: Y, NPC_ID
 *------------------------------------------*/
BUILDIN_FUNC(getnpcinfo)
{
	TBL_NPC* nd;
	if(script_hasdata(st,3))
		nd = map_id2nd(script_getnum(st,3));
	else
		nd = map_id2nd(st->oid);

	int type = script_getnum(st,2);

	if (!nd) {
		script_pushint(st,0);
		return SCRIPT_CMD_SUCCESS;
	}

	switch(type){
		case NPC_X: script_pushint(st,nd->bl.x); break;
		case NPC_Y: script_pushint(st,nd->bl.y); break;
		case NPC_ID: script_pushint(st,nd->class_); break;
		default:
			script_pushint(st,0);
			break;
	}
	return SCRIPT_CMD_SUCCESS;
}
BUILDIN_FUNC(getunitinfo)
{
	struct block_list* bl = map_id2bl(script_getnum(st,2));
	int type = script_getnum(st,3);

	if (!bl) {
		script_pushint(st,0);
		return SCRIPT_CMD_SUCCESS;
	}

	switch(type){
		case BL_MAP: script_pushstrcopy(st,map_getmapdata(bl->m)->name); break;
		case BL_X: script_pushint(st,bl->x); break;
		case BL_Y: script_pushint(st,bl->y); break;
		default:
			script_pushint(st,0);
			break;
	}
	return SCRIPT_CMD_SUCCESS;
}

/*************************************************************
Ajout de commande script: checktimer <nom event>;
=> Renvoit 1 si l'event est dans la liste des timer, 0 sinon.
*************************************************************/
BUILDIN_FUNC(checktimer)
{
	const char *event;
	TBL_PC* sd;
	event = script_getstr(st, 2);

	if (!script_rid2sd(sd))
		return SCRIPT_CMD_SUCCESS;

	check_event(st, event);
	script_pushint(st, pc_checkeventtimer(sd,event));
	return 0;
}

/*==========================================
 * Forces a unit to high jump [Grenat]
 * backsliding <unit_id>,<cells>;
 *------------------------------------------*/
BUILDIN_FUNC(unithighjump)
{
	struct block_list* src = map_id2bl(script_getnum(st,2));
	struct block_list *bl = map_id2bl(script_getnum(st,2));
	int count = script_getnum(st,3);

	nullpo_retr(1, src);
	nullpo_retr(1, bl);

	if (src->m != bl->m || status_isdead(src) || !src || !bl)
		return SCRIPT_CMD_FAILURE;
		
	int x,y, dir = unit_getdir(src);
	struct map_data *mapdata = &map[src->m];

	//Fails on noteleport maps, except for GvG and BG maps [Skotlex]
	if( mapdata->flag[MF_NOTELEPORT] &&
		!(mapdata->flag[MF_BATTLEGROUND] || mapdata_flag_gvg2(mapdata) )
	) {
		clif_skill_nodamage(src, bl, TK_HIGHJUMP, count, 1);
		return SCRIPT_CMD_FAILURE;
	} else if(dir%2) {
		//Diagonal
		x = src->x + dirx[dir]*(count*4)/3;
		y = src->y + diry[dir]*(count*4)/3;
	} else {
		x = src->x + dirx[dir]*count*2;
		y = src->y + diry[dir]*count*2;
	}

	int x1 = x + dirx[dir];
	int y1 = y + diry[dir];

	clif_skill_nodamage(src,bl,TK_HIGHJUMP,count,1);
	if( !map_count_oncell(src->m,x,y,BL_PC|BL_NPC|BL_MOB,0) && map_getcell(src->m,x,y,CELL_CHKREACH) &&
		!map_count_oncell(src->m,x1,y1,BL_PC|BL_NPC|BL_MOB,0) && map_getcell(src->m,x1,y1,CELL_CHKREACH) &&
		unit_movepos(src, x, y, 1, 0))
		clif_blown(src);

	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 * Forces a unit to run [Grenat]
 * backsliding <unit_id>,<level>;
 *------------------------------------------*/
BUILDIN_FUNC(unitrun)
{
	struct block_list *bl = map_id2bl(script_getnum(st,2));
	int count = script_getnum(st,3);

	nullpo_retr(1, bl);

	if ( status_isdead(bl) || !bl)
		return SCRIPT_CMD_FAILURE;

	sc_start4(NULL,bl,SC_RUN,100,count,unit_getdir(bl),0,0,0);

	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 * Forces a unit to backslide [Grenat]
 * backsliding <unit_id>,<cells>;
 *------------------------------------------*/
BUILDIN_FUNC(unitbacksliding)
{
	struct block_list* src = map_id2bl(script_getnum(st,2));
	struct block_list *bl = map_id2bl(script_getnum(st,2));
	int count = script_getnum(st,3);

	nullpo_retr(1, src);
	nullpo_retr(1, bl);

	if (src->m != bl->m || status_isdead(src) || !src || !bl)
		return SCRIPT_CMD_FAILURE;

	skill_blown(src,bl,count,unit_getdir(bl),(enum e_skill_blown)(BLOWN_IGNORE_NO_KNOCKBACK|BLOWN_DONT_SEND_PACKET));
	clif_skill_nodamage(src, bl, TF_BACKSLIDING, count, 1);
	clif_blown(src); // Always blow, otherwise it shows a casting animation. [Lemongrass]
	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 * Forces a unit to look towards a defined direction [Grenat]
 * unitlookdir <unit_id>,<look_dir>;
 *------------------------------------------*/
BUILDIN_FUNC(unitlookdir)
{
	struct block_list* bl = map_id2bl(script_getnum(st,2));
	int dir = script_getnum(st,3);

	if ( bl != NULL )
		unit_setdir(bl, dir);

	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 * NPC Follow [Grenat]
 * npcfollow <npc_id>,<unit_id>
 *------------------------------------------*/
BUILDIN_FUNC(npcfollow)
{
	TBL_NPC* nd = map_id2nd(script_getnum(st,2));
	struct block_list *bl = map_id2bl(script_getnum(st,3));

	if (!nd || !bl) {
		script_pushint(st,0);
		return SCRIPT_CMD_SUCCESS;
	}

	unit_walktobl(&nd->bl, bl, 2, 0);

	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 * Moves a unit visually like it throws a skill [Grenat]
 * unitmoveskill <unit_id>,<target_id>,<skill_id>;
 *------------------------------------------*/
BUILDIN_FUNC(unitmoveskill)
{
	struct block_list* bl = map_id2bl(script_getnum(st,2));
	struct block_list* src = map_id2bl(script_getnum(st,3));
	int skill_id = script_getnum(st,4);

	if ( bl != NULL ) {
		unit_refresh( bl, true );
		clif_skill_nodamage (bl, src, skill_id, 1, 0);
	}

	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 * Refresh a unit so it will show as a PJ (to be able to display skills) [Grenat]
 * unitrefresh <unit_id>; (option: walking true,false)
 *------------------------------------------*/
BUILDIN_FUNC(unitrefresh)
{
	struct block_list* bl = map_id2bl(script_getnum(st,2));
	
	if ( bl != NULL ) {
		if (script_hasdata(st, 3)) {
			if(bl->type == BL_PC)
				clif_refresh(map_id2sd(bl->id));
			unit_refresh( bl, true );
		}
		else unit_refresh( bl );
	}

	return SCRIPT_CMD_SUCCESS;
}
/*==========================================
 * Refresh a unit so it will show as a PJ (to be able to display skills) [Grenat]
 * unitrefresh <unit_id>; (option: walking true,false)
 *------------------------------------------*/
BUILDIN_FUNC(gettitleid)
{
	TBL_PC *sd;

	if(!script_rid2sd(sd))
		return SCRIPT_CMD_FAILURE;

	script_pushint(st,sd->status.title_id);
	return SCRIPT_CMD_SUCCESS;
}
BUILDIN_FUNC(settitleid)
{
	TBL_PC *sd;

	if(!script_rid2sd(sd))
		return SCRIPT_CMD_FAILURE;
	sd->status.title_id = script_getnum(st,2);
	clif_name_area(&sd->bl);
	return SCRIPT_CMD_SUCCESS;
}
#endif
#ifdef M5U00038
/*==========================================
 * Add/Save a new hat costume to bdd [Grenat]
 * addhat <item_id>;
 *------------------------------------------*/
BUILDIN_FUNC(addhat) {
	TBL_PC* sd;
	t_itemid item_id = script_getnum(st, 2);
	std::shared_ptr<item_data> id = item_db.find(item_id);
	
	if(!script_rid2sd(sd) || !id)
		return SCRIPT_CMD_FAILURE;

	if( SQL_SUCCESS != Sql_Query(mmysql_handle, "SELECT `hat_id` FROM `%s` WHERE `account_id` = '%d'", hatcostume_table, sd->status.account_id) )
		return SCRIPT_CMD_SUCCESS;

	char* data;
	int i;

	for( i = 0; i < MAX_HAT_COSTUME && SQL_SUCCESS == Sql_NextRow(mmysql_handle); ++i ) {
		Sql_GetData(mmysql_handle, 0, &data, NULL);
		unsigned long hat_ids = strtoul(data, nullptr, 10);
		if(hat_ids == item_id) {
			Sql_FreeResult(mmysql_handle);
			script_pushint(st,0);
			return SCRIPT_CMD_SUCCESS;
		}	
	}

	Sql_FreeResult(mmysql_handle);

	if( i >= MAX_HAT_COSTUME) {
		script_pushint(st,0);
		return SCRIPT_CMD_SUCCESS;
	}

	if( SQL_SUCCESS != Sql_Query(mmysql_handle, "INSERT INTO `%s`(`hat_id`, `account_id`, `char_id`) VALUES ('%d', '%d', '%d')", hatcostume_table, item_id, sd->status.account_id, sd->status.char_id) )
		return SCRIPT_CMD_SUCCESS;

	chrif_save(sd, CSAVE_NORMAL);
	script_pushint(st,1);

	return SCRIPT_CMD_SUCCESS;
}
BUILDIN_FUNC(addpet) {
	TBL_PC* sd;
	t_itemid item_id = script_getnum(st, 2);
	std::shared_ptr<item_data> id = item_db.find(item_id);
	
	if(!script_rid2sd(sd) || !id)
		return SCRIPT_CMD_FAILURE;

	if( SQL_SUCCESS != Sql_Query(mmysql_handle, "SELECT `pet_item_id` FROM `%s` WHERE `account_id` = '%d'", petcostume_table, sd->status.account_id) )
		return SCRIPT_CMD_SUCCESS;

	char* data;
	int i;

	for( i = 0; i < MAX_PET_COSTUME && SQL_SUCCESS == Sql_NextRow(mmysql_handle); ++i ) {
		Sql_GetData(mmysql_handle, 0, &data, NULL);
		unsigned long hat_ids = strtoul(data, nullptr, 10);
		if(hat_ids == item_id) {
			Sql_FreeResult(mmysql_handle);
			script_pushint(st,0);
			return SCRIPT_CMD_SUCCESS;
		}	
	}

	Sql_FreeResult(mmysql_handle);

	if( i >= MAX_PET_COSTUME) {
		script_pushint(st,0);
		return SCRIPT_CMD_SUCCESS;
	}

	if( SQL_SUCCESS != Sql_Query(mmysql_handle, "INSERT INTO `%s`(`pet_item_id`, `account_id`, `char_id`) VALUES ('%d', '%d', '%d')", petcostume_table, item_id, sd->status.account_id, sd->status.char_id) )
		return SCRIPT_CMD_SUCCESS;

	chrif_save(sd, CSAVE_NORMAL);
	script_pushint(st,1);

	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 * Delete a new hat costume to bdd [Grenat]
 * delhat <item_id>;
 *------------------------------------------*/
BUILDIN_FUNC(delhat)
{
	TBL_PC *sd;
	t_itemid item_id = script_getnum(st, 2);
	std::shared_ptr<item_data> id = item_db.find(item_id);

	if(!script_rid2sd(sd) || !id)
		return SCRIPT_CMD_FAILURE;

	if( SQL_SUCCESS != Sql_Query(mmysql_handle, "DELETE FROM `%s` WHERE `hat_id` = '%d' AND `account_id` = '%d'", hatcostume_table, item_id, sd->status.account_id) )
		return SCRIPT_CMD_SUCCESS;

	chrif_save(sd, CSAVE_NORMAL);
	 
	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 * Get a new hat costume from bdd [Grenat]
 * gethat(<item_id>); Return 1 if PJ has, 0 no
 *------------------------------------------*/
BUILDIN_FUNC(gethat)
{
	TBL_PC* sd;
	t_itemid item_id = script_getnum(st, 2);
	std::shared_ptr<item_data> id = item_db.find(item_id);
	
	if(!script_rid2sd(sd) || !id)
		return SCRIPT_CMD_FAILURE;

	if( SQL_SUCCESS != Sql_Query(mmysql_handle, "SELECT `hat_id` FROM `%s` WHERE `account_id` = '%d' AND `hat_id` = '%d'", hatcostume_table, sd->status.account_id, item_id) )
		return SCRIPT_CMD_SUCCESS;

	char* data;
	int i;

	for( i = 0; i < MAX_HAT_COSTUME && SQL_SUCCESS == Sql_NextRow(mmysql_handle); ++i ) {
		Sql_GetData(mmysql_handle, 0, &data, NULL);
		unsigned long hat_ids = strtoul(data, nullptr, 10);
		if(hat_ids == item_id) {
			Sql_FreeResult(mmysql_handle);
			script_pushint(st,1);
			return SCRIPT_CMD_SUCCESS;
		}
	}

	Sql_FreeResult(mmysql_handle);

	script_pushint(st,0);

	return SCRIPT_CMD_SUCCESS;
}
BUILDIN_FUNC(gethat2)
{
	TBL_PC* sd;
	
	if(!script_rid2sd(sd))
		return SCRIPT_CMD_FAILURE;

	char* data;
	int i = 0, count = 0;

	if( SQL_SUCCESS != Sql_Query(mmysql_handle, "SELECT `hat_id` FROM `%s` WHERE `account_id` = '%d'", hatcostume_table, sd->status.account_id) )
		return SCRIPT_CMD_SUCCESS;

	for( i = 0; i < MAX_HAT_COSTUME && SQL_SUCCESS == Sql_NextRow(mmysql_handle); ++i )
		Sql_GetData(mmysql_handle, 0, &data, NULL);

	Sql_FreeResult(mmysql_handle);

	count += i;

	if( SQL_SUCCESS != Sql_Query(mmysql_handle, "SELECT `pet_item_id` FROM `%s` WHERE `account_id` = '%d'", petcostume_table, sd->status.account_id) )
		return SCRIPT_CMD_SUCCESS;

	for( i = 0; i < MAX_PET_COSTUME && SQL_SUCCESS == Sql_NextRow(mmysql_handle); ++i )
		Sql_GetData(mmysql_handle, 0, &data, NULL);

	Sql_FreeResult(mmysql_handle);

	count += i;

	sd->hatcount = count;
	script_pushint(st,count);

	return SCRIPT_CMD_SUCCESS;
}
BUILDIN_FUNC(getpet)
{
	TBL_PC* sd;
	t_itemid item_id = script_getnum(st, 2);
	std::shared_ptr<item_data> id = item_db.find(item_id);
	
	if(!script_rid2sd(sd) || !id)
		return SCRIPT_CMD_FAILURE;

	if( SQL_SUCCESS != Sql_Query(mmysql_handle, "SELECT `pet_item_id` FROM `%s` WHERE `account_id` = '%d' AND `pet_item_id` = '%d'", petcostume_table, sd->status.account_id, item_id) )
		return SCRIPT_CMD_SUCCESS;

	char* data;
	int i;

	for( i = 0; i < MAX_HAT_COSTUME && SQL_SUCCESS == Sql_NextRow(mmysql_handle); ++i ) {
		Sql_GetData(mmysql_handle, 0, &data, NULL);
		unsigned long pet_ids = strtoul(data, nullptr, 10);
		if(pet_ids == item_id) {
			Sql_FreeResult(mmysql_handle);
			script_pushint(st,1);
			return SCRIPT_CMD_SUCCESS;
		}
	}

	Sql_FreeResult(mmysql_handle);

	script_pushint(st,0);

	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 * Select a hat costume registered in bdd (display like pets) [Grenat]
 * hatselect;
 *------------------------------------------*/
BUILDIN_FUNC(hatselect)
{
	TBL_PC *sd;

	if(!script_rid2sd(sd))
		return SCRIPT_CMD_SUCCESS;

	clif_sendhat(sd,0);

	return SCRIPT_CMD_SUCCESS;
}
/*==========================================
 * Select a hat costume registered in bdd (display like pets) [Grenat]
 * hatselect;
 *------------------------------------------*/
BUILDIN_FUNC(removehat)
{
	TBL_PC *sd;
	int type = 0;
	if(script_hasdata(st,2))
		type = script_getnum(st,2);

	if(!script_rid2sd(sd))
		return SCRIPT_CMD_SUCCESS;

	if(!type) {
		sd->hat_top = 0;
		sd->hat_mid = 0;
		sd->hat_low = 0;
		pc_setglobalreg(sd, add_str("hat_low"), 0);
		pc_setglobalreg(sd, add_str("hat_mid"), 0);
		pc_setglobalreg(sd, add_str("hat_top"), 0);
		pc_setglobalreg(sd, add_str("hat_robe"), 0);
		clif_changelook(&sd->bl, LOOK_HEAD_BOTTOM, sd->status.head_bottom);
		clif_changelook(&sd->bl, LOOK_HEAD_MID, sd->status.head_mid);
		clif_changelook(&sd->bl, LOOK_HEAD_TOP, sd->status.head_top);
		clif_changelook(&sd->bl, LOOK_ROBE, sd->status.robe);
	} else {
		sd->hat_low = 0;
		pc_setglobalreg(sd, add_str("hat_low"), 0);
		clif_changelook(&sd->bl, LOOK_HEAD_BOTTOM, sd->status.head_bottom);
	}

	return SCRIPT_CMD_SUCCESS;
}
/*==========================================
 * Select a hat costume registered in bdd (display like pets) [Grenat]
 * hatselect;
 *------------------------------------------*/
BUILDIN_FUNC(hatselect2delete)
{
	TBL_PC *sd;

	if(!script_rid2sd(sd))
		return SCRIPT_CMD_SUCCESS;

	clif_sendhat(sd,1);

	return SCRIPT_CMD_SUCCESS;
}
/*==========================================
 * Select a hat costume registered in bdd (display like pets) [Grenat]
 * hatselect;
 *------------------------------------------*/
BUILDIN_FUNC(hatselect2get)
{
	TBL_PC *sd;

	if(!script_rid2sd(sd))
		return SCRIPT_CMD_SUCCESS;

	clif_sendhat(sd,2);

	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 * Set back the hat costume from variable given when chosen (hat_top, hat_low, hat_mid) [Grenat]
 * sethat <type>,<view_id>;
 *------------------------------------------*/
BUILDIN_FUNC(sethat)
{
	TBL_PC *sd;
	int type = script_getnum(st, 2);
	int view_id = script_getnum(st, 3);

	if(!script_rid2sd(sd))
		return SCRIPT_CMD_SUCCESS;

	if(type == 0) {
		sd->hat_pet = view_id;
		struct pet_data *pd = sd->pd;
		if(pd) {
			pd->vd.class_ = view_id;
			unit_refresh(&pd->bl);
		}
		return SCRIPT_CMD_SUCCESS;
	}
	if(type == LOOK_HEAD_BOTTOM)
		sd->hat_low = view_id;
	if(type == LOOK_HEAD_MID)
		sd->hat_mid = view_id;
	if(type == LOOK_HEAD_TOP)
		sd->hat_top = view_id;
	if(type == LOOK_ROBE)
		sd->hat_robe = view_id;
	clif_changelook(&sd->bl,type,view_id);

	return SCRIPT_CMD_SUCCESS;
}
#endif

#ifdef M5U00029
/*==========================================
 * Select a hat costume registered in bdd (display like pets) [Grenat]
 * hatselect;
 *------------------------------------------*/
BUILDIN_FUNC(petmarket)
{
	TBL_PC *sd;

	if(!script_rid2sd(sd))
		return SCRIPT_CMD_SUCCESS;

	TBL_NPC *nd = map_id2nd(st->oid);

	sd->npc_shopid = nd->bl.id;
	clif_npc_market_open2(sd, nd);

	return SCRIPT_CMD_SUCCESS;
}
#endif

#ifdef M5U00049
/*==========================================
 * Get skill Information [Grenat]
 * getskillinfo(<type>,<skill>,{<lv>}):
 *------------------------------------------*/
BUILDIN_FUNC(getskillinfo)
{
	TBL_PC* sd;
	int skill_lv = 1;
	int type = script_getnum(st,2);
	int skill_id = ( script_isstring(st, 3) ? skill_name2id(script_getstr(st,3)) : script_getnum(st,3) );
	if (script_hasdata(st, 4))
		skill_lv = script_getnum(st, 4);

	if( !script_rid2sd(sd) ) {
		script_pushint(st,0);
		return SCRIPT_CMD_SUCCESS;
	}

	std::shared_ptr<s_skill_db> skill = skill_db.find((uint16)skill_id);

	if(skill) {
		switch(type){
			case SKILL_ID: script_pushint(st,skill->nameid); break;
			case SKILL_IDNAME: script_pushconststr(st,skill->name); break;
			case SKILL_MAX: script_pushint(st,skill->max); break;
			case SKILL_DUR1: script_pushint(st,skill->upkeep_time[skill_lv-1]); break;
			case SKILL_DUR2: script_pushint(st,skill->upkeep_time2[skill_lv-1]); break;
			case SKILL_NAME: script_pushconststr(st,skill->desc); break;
			case SKILL_LP: script_pushint(st,skill->lpcost); break;
			case SKILL_VISUAL1: script_pushint(st,skill->visual1); break;
			case SKILL_VISUAL2: script_pushint(st,skill->visual2); break;
			case SKILL_RELOAD: script_pushint(st,skill->reload); break;
			case SKILL_SPLASH: script_pushint(st,skill->splash[skill_lv-1]); break;
			case SKILL_SP: script_pushint(st,skill->require.sp[skill_lv-1]); break;
			case SKILL_CAST: script_pushint(st,skill->cast[skill_lv-1]); break;
			case SKILL_COOLDOWN: script_pushint(st,skill->cooldown[skill_lv-1]); break;
			case SKILL_DELAY: script_pushint(st,skill->delay[skill_lv-1]); break;
			case SKILL_ELE: script_pushint(st,skill->element[skill_lv-1]); break;
			case SKILL_RANGE: script_pushint(st,skill->range[skill_lv-1]); break;
			case SKILL_COOLDOWN2: script_pushint(st,skill->cooldown2[skill_lv-1]); break;
			case SKILL_ATTACHED1: script_pushconststr(st,skill->skillattached); break;
			case SKILL_ATTACHED2: script_pushconststr(st,skill->skillattached2); break;
			case SKILL_ATTACHED3: script_pushconststr(st,skill->skillattached3); break;
			case SKILL_TYPE: script_pushint(st,(int)skill->skill_type); break;
			case SKILL_INF: script_pushint(st,(int)skill->inf); break;
			default: script_pushint(st,0); break;
		}
	} else
		script_pushint(st,0);

	return SCRIPT_CMD_SUCCESS;
}
/*==========================================
 * Get skill Information [Grenat]
 * getskillid(<skill_name>):
 *------------------------------------------*/
BUILDIN_FUNC(getskillid)
{
	TBL_PC* sd;

	if( !script_rid2sd(sd) )
		return SCRIPT_CMD_SUCCESS;

	script_pushint(st, skill_name2id(script_getstr(st,2)));

	return SCRIPT_CMD_SUCCESS;
}
#endif

#ifdef M5U00037
/// Sets the group ID of the player to 1 (Super Player) [Grenat]
///
/// setgroupid({<char_id>}) -> <int>
BUILDIN_FUNC(setgroupid)
{
	TBL_PC* sd;

	if (!script_charid2sd(2,sd))
		return SCRIPT_CMD_FAILURE;
	if(sd)
		sd->group_id = 1;
	return SCRIPT_CMD_SUCCESS;
}
#endif
#ifdef M5U00039
/// Get value of xp for a specified Race [Grenat]
///
/// getpcexprace(<RC_>)
BUILDIN_FUNC(getpcexprace)
{
	TBL_PC* sd;
	int race_id = script_getnum(st, 2);

	if (!script_rid2sd(sd))
		return SCRIPT_CMD_FAILURE;
	if(sd)
		script_pushint(st, sd->indexed_bonus.expaddrace[race_id]);
	return SCRIPT_CMD_SUCCESS;
}
/// Get value of xp for a specified Size [Grenat]
///
/// getpcexpsize(<SZ_>)
BUILDIN_FUNC(getpcexpsize)
{
	TBL_PC* sd;
	int size_id = script_getnum(st, 2);

	if (!script_rid2sd(sd))
		return SCRIPT_CMD_FAILURE;
	if(sd)
		script_pushint(st, sd->special_state.exp_add_size[size_id]);
	return SCRIPT_CMD_SUCCESS;
}
/// Get value of xp for a specified Class [Grenat]
///
/// getpcexpclass(<Class_>)
BUILDIN_FUNC(getpcexpclass)
{
	TBL_PC* sd;
	int class_id = script_getnum(st, 2);

	if (!script_rid2sd(sd))
		return SCRIPT_CMD_FAILURE;
	if(sd)
		script_pushint(st, sd->indexed_bonus.expaddclass[class_id]);
	return SCRIPT_CMD_SUCCESS;
}
/// Get value of xp for a specified Element [Grenat]
///
/// getpcexpelement(<Ele_>)
BUILDIN_FUNC(getpcexpelement)
{
	TBL_PC* sd;
	int element_id = script_getnum(st, 2);

	if (!script_rid2sd(sd))
		return SCRIPT_CMD_FAILURE;
	if(sd)
		script_pushint(st, sd->special_state.exp_add_element[element_id]);
	return SCRIPT_CMD_SUCCESS;
}
#endif
#ifdef E_GEPARDSHIELD
BUILDIN_FUNC(get_unique_id)
{
	struct map_session_data* sd;
	if(script_hasdata(st, 2))
		sd = map_id2sd(script_getnum(st, 2));
	else if (!script_rid2sd(sd)) {
		script_pushint(st, 0);
		return SCRIPT_CMD_FAILURE;
	}

	script_pushint(st, sd?(session[sd->fd]->gepard_info.unique_id):0);

	return SCRIPT_CMD_SUCCESS;
}
#endif
#ifdef M5U00046
/*==========================================
 * Set poring ball mode [Grenat]
 * setporingball;
 *------------------------------------------*/
BUILDIN_FUNC(setporingball)
{
	struct map_session_data* sd;

	if (!script_rid2sd(sd))
		return SCRIPT_CMD_FAILURE;

	sd->ballx = 1;

	return SCRIPT_CMD_SUCCESS;
}
/*==========================================
 * Get X value of clicked cell [Grenat]
 * getporingballx();
 *------------------------------------------*/
BUILDIN_FUNC(getporingballx)
{
	struct map_session_data* sd;

	if (!script_rid2sd(sd))
	{
		script_pushint(st, 0);
		return SCRIPT_CMD_FAILURE;
	}

	script_pushint(st, sd->ballx);

	return SCRIPT_CMD_SUCCESS;
}
/*==========================================
 * Get Y value of clicked cell [Grenat]
 * getporingballx();
 *------------------------------------------*/
BUILDIN_FUNC(getporingbally)
{
	struct map_session_data* sd;

	if (!script_rid2sd(sd))
	{
		script_pushint(st, 0);
		return SCRIPT_CMD_FAILURE;
	}

	script_pushint(st, sd->bally);

	return SCRIPT_CMD_SUCCESS;
}
/*==========================================
 * Remove poring ball mode to save ressources [Grenat]
 * removeporingball;
 *------------------------------------------*/
BUILDIN_FUNC(removeporingball)
{
	struct map_session_data* sd;

	if (!script_rid2sd(sd))
		return SCRIPT_CMD_FAILURE;

	sd->ballx = 0;
	sd->bally = 0;

	return SCRIPT_CMD_SUCCESS;
}
/*==========================================
 * Get direction the unit is looking [Grenat]
 * getunitdir <unit_id/account_id>;
 *------------------------------------------*/
BUILDIN_FUNC(getunitdir)
{
	struct block_list* bl = map_id2bl(script_getnum(st,2));
	
	if (!bl) {
		script_pushint(st, 0);
		return SCRIPT_CMD_FAILURE;
	}

	script_pushint(st, unit_getdir(bl));

	return SCRIPT_CMD_SUCCESS;
}
/*==========================================
 * Get direction the unit is looking [Grenat]
 * unitmove <unit_id/account_id>,<x>,<y>;
 *------------------------------------------*/
BUILDIN_FUNC(unitmove)
{
	struct block_list* bl = map_id2bl(script_getnum(st,2));
	int x = script_getnum(st, 3);
	int y = script_getnum(st, 4);
	
	if (!bl) {
		script_pushint(st, 0);
		return SCRIPT_CMD_FAILURE;
	}

	unit_movepos(bl, x, y, 1, 0);
	clif_blown(bl);

	return SCRIPT_CMD_SUCCESS;
}
/*==========================================
 * Get information if a pc is sit or not [Grenat]
 * ispcsit <account_id>;
 *------------------------------------------*/
BUILDIN_FUNC(ispcsit)
{
	struct map_session_data* sd;

	if( !script_hasdata(st, 2) )
		script_rid2sd(sd);
	else
		sd = map_id2sd( script_getnum(st, 2) );

	if(sd) {
		if( pc_issit(sd) )
			script_pushint(st, 1);
		else
			script_pushint(st, 0);
	}

	return SCRIPT_CMD_SUCCESS;
}
#endif
#ifdef M5U00051
/*==========================================
 * Return true level for a monster depending on HP/ATK values
 * getLvStatByMob <mob_id>;
 *------------------------------------------*/
BUILDIN_FUNC(getLvStatByMob)
{
	struct map_session_data* sd;
	int mob_id = script_getnum(st, 2);

	if (!script_rid2sd(sd))
		return SCRIPT_CMD_FAILURE;

	if (mob_id >= 0 && !mobdb_checkid(mob_id)) {
		ShowWarning("buildin_monster: Attempted to spawn non-existing monster class %d\n", mob_id);
		return SCRIPT_CMD_FAILURE;
	}
	std::shared_ptr<s_mob_db> mob = mob_db.find(mob_id);
	if(!mob) {
		script_pushint(st, 0);
		return SCRIPT_CMD_FAILURE;
	}

	int mob_hp_lvl = 0;
	int mob_atk_lvl = 0;
	// Find the true level for mobs with this amount of HP
	int search = 0;
	for (int i = 1;search < mob->status.max_hp;i++) {
		mob_hp_lvl++;
		search = hp_for_level(i);
		if (i >= 250)
			search = mob->status.max_hp;
	}
	// Find the true level for mobs with this amount of (average) ATK
	search = 0;
	int average_atk = (mob->status.rhw.atk + mob->status.rhw.atk2) / 2;
	for (int i = 1; search < average_atk;i++) {
		mob_atk_lvl++;
		search = atk_for_level(i);
		if (i >= 250)
			search = average_atk;
	}

	int true_level = (mob_hp_lvl + mob_atk_lvl)/2;
	script_pushint(st, true_level);

	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 * Return stats for a monster with a specific level
 * getMobStatByLv <mob_id>, <mob_lv>, <mob_type>, <flag>;
 *------------------------------------------*/
BUILDIN_FUNC(getMobStatByLv)
{
	int mob_id = script_getnum(st, 2);
	int mob_lv = script_getnum(st, 3);
	int mob_tp = script_getnum(st, 4);
	int mob_sf = script_getnum(st, 5);

	if (mob_id >= 0 && !mobdb_checkid(mob_id)) {
		ShowWarning("buildin_monster: Attempted to spawn non-existing monster class %d\n", mob_id);
		return SCRIPT_CMD_FAILURE;
	}

	if (mob_sf) {
		struct map_session_data* sd;
		if (!script_rid2sd(sd))
			return SCRIPT_CMD_FAILURE;

		struct mob_data* md = map_id2md(mob_once_spawn(sd, sd->bl.m, 0, 0, "--ja--", mob_id, 1, "", 0, AI_ATTACK, (special_flag) mob_sf));

		mapreg_setreg(reference_uid(add_str("$@MOBS_STATS"), 0), md->status.hp);
		mapreg_setreg(reference_uid(add_str("$@MOBS_STATS"), 1), md->status.rhw.atk);
		mapreg_setreg(reference_uid(add_str("$@MOBS_STATS"), 2), md->status.rhw.atk2);
		mapreg_setreg(reference_uid(add_str("$@MOBS_STATS"), 3), md->status.def);
		mapreg_setreg(reference_uid(add_str("$@MOBS_STATS"), 4), md->status.mdef);
		mapreg_setreg(reference_uid(add_str("$@MOBS_STATS"), 5), md->status.str);
		mapreg_setreg(reference_uid(add_str("$@MOBS_STATS"), 6), md->status.agi);
		mapreg_setreg(reference_uid(add_str("$@MOBS_STATS"), 7), md->status.vit);
		mapreg_setreg(reference_uid(add_str("$@MOBS_STATS"), 8), md->status.int_);
		mapreg_setreg(reference_uid(add_str("$@MOBS_STATS"), 9), md->status.dex);
		mapreg_setreg(reference_uid(add_str("$@MOBS_STATS"), 10), md->status.luk);
		mapreg_setreg(reference_uid(add_str("$@MOBS_STATS"), 11), md->status.hit);
		mapreg_setreg(reference_uid(add_str("$@MOBS_STATS"), 12), md->status.flee);
		mapreg_setreg(reference_uid(add_str("$@MOBS_STATS"), 13), md->status.cri);
		mapreg_setreg(reference_uid(add_str("$@MOBS_STATS"), 14), md->status.flee2);
		mapreg_setreg(reference_uid(add_str("$@MOBS_STATS"), 15), md->status.speed);
		mapreg_setreg(reference_uid(add_str("$@MOBS_STATS"), 16), md->status.adelay);
		mapreg_setreg(reference_uid(add_str("$@MOBS_STATS"), 17), md->status.matk_min);
		mapreg_setreg(reference_uid(add_str("$@MOBS_STATS"), 18), md->status.matk_max);
		mapreg_setreg(reference_uid(add_str("$@MOBS_STATS"), 19), md->status.amotion);
		mapreg_setreg(reference_uid(add_str("$@MOBS_STATS"), 20), md->status.dmotion);
		mapreg_setreg(reference_uid(add_str("$@MOBS_STATS"), 21), md->base_exp);
		mapreg_setreg(reference_uid(add_str("$@MOBS_STATS"), 22), md->job_exp);
		mapreg_setreg(reference_uid(add_str("$@MOBS_STATS"), 23), md->status.sp);
	#ifdef M5G00013
		mapreg_setreg(reference_uid(add_str("$@MOBS_STATS"), 24), md->zeny);
	#endif
		mapreg_setreg(reference_uid(add_str("$@MOBS_STATS"), 25), md->level);
	} else {
		struct mob_data_temp mob_stats = mob_stat_by_lv(mob_id, mob_lv, mob_tp);
		mapreg_setreg(reference_uid(add_str("$@MOBS_STATS"), 0), mob_stats.hp);
		mapreg_setreg(reference_uid(add_str("$@MOBS_STATS"), 1), mob_stats.atk1);
		mapreg_setreg(reference_uid(add_str("$@MOBS_STATS"), 2), mob_stats.atk2);
		mapreg_setreg(reference_uid(add_str("$@MOBS_STATS"), 3), mob_stats.def);
		mapreg_setreg(reference_uid(add_str("$@MOBS_STATS"), 4), mob_stats.mdef);
		mapreg_setreg(reference_uid(add_str("$@MOBS_STATS"), 5), mob_stats.str);
		mapreg_setreg(reference_uid(add_str("$@MOBS_STATS"), 6), mob_stats.agi);
		mapreg_setreg(reference_uid(add_str("$@MOBS_STATS"), 7), mob_stats.vit);
		mapreg_setreg(reference_uid(add_str("$@MOBS_STATS"), 8), mob_stats.int_);
		mapreg_setreg(reference_uid(add_str("$@MOBS_STATS"), 9), mob_stats.dex);
		mapreg_setreg(reference_uid(add_str("$@MOBS_STATS"), 10), mob_stats.luk);
		mapreg_setreg(reference_uid(add_str("$@MOBS_STATS"), 11), mob_stats.hit);
		mapreg_setreg(reference_uid(add_str("$@MOBS_STATS"), 12), mob_stats.flee);
		mapreg_setreg(reference_uid(add_str("$@MOBS_STATS"), 13), mob_stats.cri);
		mapreg_setreg(reference_uid(add_str("$@MOBS_STATS"), 14), mob_stats.flee2);
		mapreg_setreg(reference_uid(add_str("$@MOBS_STATS"), 15), mob_stats.speed);
		mapreg_setreg(reference_uid(add_str("$@MOBS_STATS"), 16), mob_stats.adelay);
		mapreg_setreg(reference_uid(add_str("$@MOBS_STATS"), 17), mob_stats.matk1);
		mapreg_setreg(reference_uid(add_str("$@MOBS_STATS"), 18), mob_stats.matk2);
		mapreg_setreg(reference_uid(add_str("$@MOBS_STATS"), 19), mob_stats.amotion);
		mapreg_setreg(reference_uid(add_str("$@MOBS_STATS"), 20), mob_stats.dmotion);
		mapreg_setreg(reference_uid(add_str("$@MOBS_STATS"), 21), mob_stats.bxp);
		mapreg_setreg(reference_uid(add_str("$@MOBS_STATS"), 22), mob_stats.jxp);
		mapreg_setreg(reference_uid(add_str("$@MOBS_STATS"), 23), mob_stats.sp);
	#ifdef M5G00013
		mapreg_setreg(reference_uid(add_str("$@MOBS_STATS"), 24), mob_stats.zeny);
	#endif
		mapreg_setreg(reference_uid(add_str("$@MOBS_STATS"), 25), mob_lv);
	}

	return SCRIPT_CMD_SUCCESS;
}
#endif

#ifdef M5U00049
/*==========================================
 * Adds/Removes a kugan skill to read its script [Grenat]
 * addkugan(<skill_id>,<skill_lv>);
 *------------------------------------------*/
BUILDIN_FUNC(addkugan)
{
	struct map_session_data* sd;
	int skill_id = script_getnum(st,2);
	int skill_lv = script_getnum(st,3);
	int i;
	int already_learned = -1;
	uint16 idx = 0;

	if (!script_rid2sd(sd))
		return SCRIPT_CMD_FAILURE;

	if ((idx = skill_get_index(skill_id)) == 0) {
		ShowError("pc_checkskill: Invalid skill id %d (char_id=%d).\n", skill_id, sd->status.char_id);
		return 0;
	}
	if(skill_id && skill_lv) {
		for(i = 0; i < sd->status.kugan_num; i++) {
			if(sd->status.kugans[i] == skill_id)
				already_learned = i;
		}
			
		if(already_learned >= 0) {
			sd->status.kuganslevel[already_learned] = skill_lv;
		} else {
			sd->status.kugans[sd->status.kugan_num] = skill_id;
			sd->status.kuganslevel[sd->status.kugan_num] = skill_lv;
			sd->status.kugan_num++;
		}

		uint16 idx = skill_get_index(skill_id);
		sd->status.skill[idx].id   = skill_id;
		sd->status.skill[idx].lv   = skill_lv;
		sd->status.skill[idx].flag = SKILL_FLAG_PERMANENT;
		clif_addskill(sd,skill_id);
		script_pushint(st, 1);
		return SCRIPT_CMD_SUCCESS;
	} else {
		script_pushint(st, 0);
		return SCRIPT_CMD_FAILURE;
	}

	script_pushint(st, 1);
	return SCRIPT_CMD_SUCCESS;
}
BUILDIN_FUNC(getkuganlv)
{
	struct map_session_data* sd;
	int skill_id = script_getnum(st,2);
	int i;

	if (!script_rid2sd(sd))
		return SCRIPT_CMD_FAILURE;

	if(skill_id) {
		for(i = 0; i < sd->status.kugan_num; i++) {
			if(sd->status.kugans[i] == skill_id) {
				script_pushint(st, sd->status.kuganslevel[i]);
				return SCRIPT_CMD_SUCCESS;
			}
		}
	}
	script_pushint(st, 0);
	return SCRIPT_CMD_SUCCESS;
				
}
BUILDIN_FUNC(isshowexp)
{
	struct map_session_data* sd;

	if (!script_rid2sd(sd))
		return SCRIPT_CMD_FAILURE;
	if(sd->state.showexp)
		script_pushint(st, 1);
	else
		script_pushint(st, 0);
	return SCRIPT_CMD_SUCCESS;
}
BUILDIN_FUNC(clearkugan)
{
	struct map_session_data* sd;
	int i;

	if (!script_rid2sd(sd))
		return SCRIPT_CMD_FAILURE;

	for(i = 0; i < sd->status.kugan_num; i++) {
		if(i == sd->status.kugan_num-1) {
			uint16 idx = skill_get_index(sd->status.kugans[i]);
			sd->status.skill[idx].id   = 0;
			sd->status.skill[idx].lv   = 0;
		}
		sd->status.kugans[i] = 0;
		sd->status.kuganslevel[i] = 0;
	}
	sd->status.kugan_num = 0;

	return SCRIPT_CMD_SUCCESS;
}
BUILDIN_FUNC(ClifNoKugan)
{
	struct map_session_data* sd;
	int i;

	if (!script_rid2sd(sd))
		return SCRIPT_CMD_FAILURE;

	for(i = 0; i < sd->status.kugan_num; i++)
		clif_deleteskill2(sd,sd->status.kugans[i]);

	return SCRIPT_CMD_SUCCESS;
}
uint16 skill_attached2id2(const char* name) {
	if (name == nullptr)
		return 0;

	for (const auto &it : skill_db) {
		if (strcmpi(it.second->skillattached2, name) == 0)
			return it.first;
	}

	return 0;
}

BUILDIN_FUNC(resetkugan)
{
	struct map_session_data* sd;
	int i, i2;

	if (!script_rid2sd(sd))
		return SCRIPT_CMD_FAILURE;

	for(i = 0; i < sd->status.kugan_num; i++) {
		uint16 idx = skill_get_index(sd->status.kugans[i]);
		std::shared_ptr<s_skill_db> skill = skill_db.find(sd->status.kugans[i]);
		if(skill) {
			clif_deleteskill2(sd,sd->status.kugans[i]);
			sd->status.skill[idx].id   = 0;
			sd->status.skill[idx].lv   = 0;
			sd->status.kugans[i] = 0;
			sd->status.kuganslevel[i] = 0;
			std::string sk1 = skill->skillattached;
			std::string sk2 = skill->skillattached2;
			std::string sk3 = skill->skillattached3;
			std::string sk4 = skill->skillattached4;
			if(i2 = skill_name2id(sk1.c_str()))
				clif_deleteskill2(sd, i2);
			if(i2 = skill_name2id(sk2.c_str()))
				clif_deleteskill2(sd, i2);
			if(i2 = skill_name2id(sk3.c_str()))
				clif_deleteskill2(sd, i2);
			if(i2 = skill_name2id(sk4.c_str()))
				clif_deleteskill2(sd, i2);
		}
	}
	for(i = 0; i <= sd->status.kugansPerm_num; i++) {
		sd->status.kugansPerm[i] = 0;
		sd->status.kugansPermLv[i] = 0;
		sd->status.kugansPerm2[i] = 0;
		sd->status.kugansPermLv2[i] = 0;
	}
	sd->status.kugansPerm_num = 0;
	sd->status.kugan_num = 0;
	status_calc_pc(sd, SCO_FORCE);
	return SCRIPT_CMD_SUCCESS;
}
BUILDIN_FUNC(kugangetskill)
{
	struct map_session_data* sd;
	int skill_id = ( script_isstring(st, 2) ? skill_name2id(script_getstr(st,2)) : script_getnum(st,2) );
	int level = script_getnum(st,3);
	int i = 0, i2 = 0, alreadyhave = 0;
	if(script_hasdata(st,4))
		alreadyhave = script_getnum(st,4);

	if (!script_rid2sd(sd))
		return SCRIPT_CMD_FAILURE;

	uint16 idx = 0;

	if (!skill_id || !(idx = skill_get_index(skill_id))) {
		ShowError("pc_skill: Skill with id %d does not exist in the skill database\n", skill_id);
		return false;
	}

	if(alreadyhave) {
		int lv = 0;

		auto entry = skill_tree_db.get_skill_data(sd->status.class_, skill_id);
		if(entry)
			lv = pc_checkskill(sd,skill_id);
		pc_setglobalreg(sd, reference_uid(add_str("KuganAddtoSkill"), skill_id), lv + (pc_readglobalreg(sd, reference_uid(add_str("KuganAddtoSkill"), skill_id))));
		level += lv;
	}

	if(level > 20) level = 20; // Maximum Lv possible
	sd->status.skill[idx].id   = skill_id;
	sd->status.skill[idx].lv   = level;
	sd->status.skill[idx].flag = SKILL_FLAG_PERMANENT;
	clif_addskill(sd,skill_id);

	return SCRIPT_CMD_SUCCESS;
}
BUILDIN_FUNC(isskillintree)
{
	struct map_session_data* sd;
	int skill_id = ( script_isstring(st, 2) ? skill_name2id(script_getstr(st,2)) : script_getnum(st,2) );
	uint16 idx = 0;

	if (!script_rid2sd(sd))
		return SCRIPT_CMD_FAILURE;

	if ((idx = skill_get_index(skill_id)) == 0) {
		ShowError("pc_checkskill: Invalid skill id %d (char_id=%d).\n", skill_id, sd->status.char_id);
		return 0;
	}
	unsigned short skid = 0;
	unsigned short skid2 = 0;
	if (pc_checkskill(sd,RG_PLAGIARISM))
		skid = static_cast<unsigned short>(pc_readglobalreg(sd, add_str(SKILL_VAR_PLAGIARISM)));
	if (pc_checkskill(sd,SC_REPRODUCE))
		skid2 = static_cast<unsigned short>(pc_readglobalreg(sd, add_str(SKILL_VAR_REPRODUCE)));

	auto skill = skill_tree_db.get_skill_data(sd->status.class_, skill_id);

	if(skill)
		script_pushint(st, (skill->skill_id == skill_id || skid2 == skill_id || skid == skill_id) ? 1 : 0);
	else
		script_pushint(st, 0);

	return SCRIPT_CMD_SUCCESS;

}
BUILDIN_FUNC(addkuganperm)
{
	struct map_session_data* sd;
	int skill_id = ( script_isstring(st, 2) ? skill_name2id(script_getstr(st,2)) : script_getnum(st,2) );
	int skill_lv = script_getnum(st,3);
	int i;
	uint16 idx = 0;

	if (!script_rid2sd(sd))
		return SCRIPT_CMD_FAILURE;

	if ((idx = skill_get_index(skill_id)) == 0) {
		ShowError("pc_checkskill: Invalid skill id %d (char_id=%d).\n", skill_id, sd->status.char_id);
		return 0;
	}
	for(i = 0; i <= sd->status.kugansPerm_num; i++) {
		if(sd->status.kugansPerm[i] == 0) {
			sd->status.kugansPerm[i] = skill_id;
			sd->status.kugansPermLv[i] = skill_lv;
			sd->status.kugansPerm_num++;
			return SCRIPT_CMD_SUCCESS;
		}
	}

	return SCRIPT_CMD_SUCCESS;
}
BUILDIN_FUNC(getkuganperm)
{
	struct map_session_data* sd;
	int skill_id = ( script_isstring(st, 2) ? skill_name2id(script_getstr(st,2)) : script_getnum(st,2) );
	int i;

	if (!script_rid2sd(sd))
		return SCRIPT_CMD_FAILURE;

	for(i = 0; i <= sd->status.kugansPerm_num; i++) {
		if(sd->status.kugansPerm[i] == skill_id) {
			script_pushint(st, sd->status.kugansPermLv[i]);
			return SCRIPT_CMD_SUCCESS;
		}
	}
	script_pushint(st, 0);

	return SCRIPT_CMD_SUCCESS;
}
BUILDIN_FUNC(getkuganlist)
{
	struct map_session_data* sd;
	int skill_id = script_getnum(st,2) ;

	if (!script_rid2sd(sd))
		return SCRIPT_CMD_FAILURE;

	script_pushint(st, sd->status.kugansPerm[skill_id]);

	return SCRIPT_CMD_SUCCESS;
}
BUILDIN_FUNC(resetkuganperm)
{
	struct map_session_data* sd;
	int i;

	if (!script_rid2sd(sd))
		return SCRIPT_CMD_FAILURE;

	for(i = 0; i <= sd->status.kugansPerm_num; i++) {
		sd->status.kugansPerm[i] = 0;
		sd->status.kugansPermLv[i] = 0;
	}

	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(addkuganperm2)
{
	struct map_session_data* sd;
	int skill_id = ( script_isstring(st, 2) ? skill_name2id(script_getstr(st,2)) : script_getnum(st,2) );
	int skill_lv = script_getnum(st,3);
	int i;
	uint16 idx = 0;

	if (!script_rid2sd(sd))
		return SCRIPT_CMD_FAILURE;

	if ((idx = skill_get_index(skill_id)) == 0) {
		ShowError("pc_checkskill: Invalid skill id %d (char_id=%d).\n", skill_id, sd->status.char_id);
		return 0;
	}
	for(i = 0; i <= sd->status.kugansPerm_num; i++) {
		if(sd->status.kugansPerm2[i] == 0) {
			sd->status.kugansPerm2[i] = skill_id;
			sd->status.kugansPermLv2[i] = skill_lv;
			sd->status.kugansPermMax2[i] = skill_lv;
			return SCRIPT_CMD_SUCCESS;
		}
	}

	return SCRIPT_CMD_SUCCESS;
}
BUILDIN_FUNC(getkuganperm2)
{
	struct map_session_data* sd;
	int skill_id = ( script_isstring(st, 2) ? skill_name2id(script_getstr(st,2)) : script_getnum(st,2) );
	int type = 1;
	if(script_hasdata(st,3))
		type = script_getnum(st,3);
	int i;

	if (!script_rid2sd(sd))
		return SCRIPT_CMD_FAILURE;

	if(type == 3) {
			script_pushint(st, sd->status.kugansPerm_num);
			return SCRIPT_CMD_SUCCESS;
	}
	for(i = 0; i <= sd->status.kugansPerm_num; i++) {
		if(sd->status.kugansPerm2[i] == skill_id) {
			if(type == 1) script_pushint(st, sd->status.kugansPermLv2[i]);
			if(type == 2) script_pushint(st, sd->status.kugansPermMax2[i]);
			return SCRIPT_CMD_SUCCESS;
		}
	}
	script_pushint(st, 0);

	return SCRIPT_CMD_SUCCESS;
}
BUILDIN_FUNC(resetkuganperm2)
{
	struct map_session_data* sd;
	int i;
	if (!script_rid2sd(sd))
		return SCRIPT_CMD_FAILURE;

	for(i = 0; i <= sd->status.kugansPerm_num; i++) {
		sd->status.kugansPerm2[i] = 0;
		sd->status.kugansPermLv2[i] = 0;
	}

	return SCRIPT_CMD_SUCCESS;
}
#endif

#ifdef M5U00049
BUILDIN_FUNC(ress)
{
	struct map_session_data* sd;
	int skill_id = ( script_isstring(st, 2) ? skill_name2id(script_getstr(st,2)) : script_getnum(st,2) );

	if (!script_rid2sd(sd))
		return SCRIPT_CMD_FAILURE;

	clif_deleteskill2(sd,skill_id);
	return SCRIPT_CMD_SUCCESS;
}
#endif
#ifdef M5J00006
/*==========================================
 * Return summon class ID [Grenat]
 * getsummoninfo(<type>);
 * 1: ID, 2: Class
 *------------------------------------------*/
BUILDIN_FUNC(getsummoninfo)
{
	TBL_PC *sd;
	int info = script_getnum(st,2);

	if (!script_rid2sd(sd)) {
		script_pushint(st, 0);
		return SCRIPT_CMD_SUCCESS;
	}
	if(!map_id2bl(sd->summon_id)) {
		script_pushint(st, 0);
		return SCRIPT_CMD_SUCCESS;
	}

	switch (info) {
		case 1: script_pushint(st, sd->summon_id); break;
		case 2: script_pushint(st, ((TBL_MOB*)map_id2bl(sd->summon_id))->mob_id); break;
	}

	return SCRIPT_CMD_SUCCESS;
}
#endif
#ifdef M5U00053
/*==========================================
 * Delete all dailies/weekly/monthly for everyone in the bdd [Grenat]
 * resetachievement <type>; (0 = Daily, 1 = Weekly, 2 = Monthly)
 *------------------------------------------*/
BUILDIN_FUNC(resetachievement)
{
	int type = script_getnum(st, 2);
	int j = 0, i, IDmin = 0, IDmax = 0;
	switch (type) {
		case 0: IDmin = 111001; IDmax = 111004; break;
		case 1: IDmin = 112001; IDmax = 112006; break;
		case 2: IDmin = 113001; IDmax = 113005; break;
	}

	for(i = IDmin; i <= IDmax; i++)
		if( SQL_SUCCESS != Sql_Query(mmysql_handle, "DELETE FROM `%s` WHERE `id` = '%d'", achievement_table, i) )
			j++;

	struct map_session_data* sd;
	struct s_mapiterator* iter = mapit_getallusers();
	for (sd = (TBL_PC*)mapit_first(iter); mapit_exists(iter); sd = (TBL_PC*)mapit_next(iter)) {
		switch(type) {
			case 0:
				achievement_remove(sd, 111001);
				achievement_remove(sd, 111002);
				achievement_remove(sd, 111003);
				achievement_remove(sd, 111004);
				break;
			case 1:
				achievement_remove(sd, 112001);
				achievement_remove(sd, 112002);
				achievement_remove(sd, 112003);
				achievement_remove(sd, 112004);
				achievement_remove(sd, 112005);
				achievement_remove(sd, 112006);
				break;
			case 2:
				achievement_remove(sd, 113001);
				achievement_remove(sd, 113002);
				achievement_remove(sd, 113003);
				achievement_remove(sd, 113004);
				achievement_remove(sd, 113005);
				break;
		}
	}
	mapit_free(iter);
	return SCRIPT_CMD_SUCCESS;
}
BUILDIN_FUNC(votereward)
{
	if( SQL_SUCCESS != Sql_Query(mmysql_handle, "SELECT `account_id`,`quantity` FROM `%s` WHERE `imported` = '0'", cp_rotopservlog_table) )
		return SCRIPT_CMD_SUCCESS;

	char* data;
	unsigned long account_id = 0;
	unsigned long val = 0;

	while( SQL_SUCCESS == Sql_NextRow(mmysql_handle) ) {
		Sql_GetData(mmysql_handle, 0, &data, NULL); account_id = strtoul(data, nullptr, 10);
		Sql_GetData(mmysql_handle, 1, &data, NULL); val = strtoul(data, nullptr, 10);
		if(account_id) {
			if(val >= 100) {
				if( SQL_SUCCESS != Sql_Query(mmysql_handle, "INSERT INTO `%s`(`char_id`, `account_id`, `id`, `count1`, `count2`, `count3`, `count4`, `count5`, `count6`, `count7`, `count8`, `count9`, `count10`, `completed`, `rewarded`) VALUES ('0', '%d', '113005', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', 'NULL', 'NULL')", achievement_table, account_id) )
					continue;
			}
			if( SQL_SUCCESS != Sql_Query(mmysql_handle, "INSERT INTO `%s`(`quantity`, `cost`, `account_id`, `purchase_date`, `statut`) VALUES ('%d', '0', '%d', '2021-11-30 07:47:23', '0')", cp_purchaselog_table, val, account_id) )
				continue;
		}
	}
	Sql_Query(mmysql_handle, "REPLACE INTO `%s` (`imported`) VALUES ('1')",cp_rotopservlog_table);

	Sql_FreeResult(mmysql_handle);

	return SCRIPT_CMD_SUCCESS;
}

/**
 * questinfo2(<ID>{,<char_id>});
 **/
BUILDIN_FUNC(questinfo2)
{
	struct map_session_data *sd;
	std::shared_ptr<s_quest_db> qi = quest_search(script_getnum(st, 2));
	int j;

	if (!script_charid2sd(3,sd) || !qi)
		return SCRIPT_CMD_FAILURE;

	for( j = 0; j < qi->objectives.size(); j++ ) {
		pc_setreg(sd,reference_uid(add_str("@mob_id"), j),qi->objectives[j]->mob_id);
		pc_setreg(sd,reference_uid(add_str("@mob_count"), j),qi->objectives[j]->count);
	}

	return SCRIPT_CMD_SUCCESS;
}
BUILDIN_FUNC(getpcparam)
{
	struct map_session_data *sd;
	int type = script_getnum(st, 2);

	if (!script_charid2sd(3,sd))
		return SCRIPT_CMD_FAILURE;
	struct status_data* status = status_get_status_data(&sd->bl);

	switch (type) {
		case 1: script_pushint(st, status->str); break;
		case 2: script_pushint(st, status->agi); break;
		case 3: script_pushint(st, status->vit); break;
		case 4: script_pushint(st, status->dex); break;
		case 5: script_pushint(st, status->int_); break;
		case 6: script_pushint(st, status->luk); break;
		case 7: script_pushint(st, status->matk_min); break;
		case 8: script_pushint(st, status->matk_max); break;
		case 9: script_pushint(st, status->batk); break;
		case 11: script_pushint(st, status->def2); break;
		case 12: script_pushint(st, status->def); break;
		case 13: script_pushint(st, status->mdef); break;
		case 14: script_pushint(st, status->mdef2); break;
		case 15: script_pushint(st, status->rhw.range); break;
		case 16: script_pushint(st, status->rhw.ele); break;
	}

	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 * Delete all Daily Quests for everyone in the bdd [Grenat]
 * resetdailyquest;
 *------------------------------------------*/
BUILDIN_FUNC(resetdailyquest)
{
	if( SQL_SUCCESS != Sql_Query(mmysql_handle, "DELETE FROM `%s` WHERE `quest_id` >= '49300' AND `quest_id` <= '49400'", quest_table) )
		return SCRIPT_CMD_SUCCESS;
	return SCRIPT_CMD_SUCCESS;
}
#endif
#ifdef M5U00004
/*==========================================
 * Get elite flag from a RID/GID [Grenat]
 * hasflag(<unit_id>,<s_flag>);
 *------------------------------------------*/
BUILDIN_FUNC(hasflag)
{
	struct block_list* bl;

	if(!script_rid2bl(2,bl))
		return SCRIPT_CMD_FAILURE;

	TBL_MOB* md = map_id2md(bl->id);

	if(md && md->s_flag&(script_getnum(st,3)))
		script_pushint(st, 1);
	else 
		script_pushint(st, 0);

	return SCRIPT_CMD_SUCCESS;
}
#endif
#ifdef M5U00058
BUILDIN_FUNC(ispcnight)
{
	struct map_session_data *sd;

	if (!script_charid2sd(2,sd))
		return SCRIPT_CMD_FAILURE;

	if(sd->state.night)
		script_pushint(st, 1);
	else
		script_pushint(st, 0);

	return SCRIPT_CMD_SUCCESS;
}
BUILDIN_FUNC(setnight)
{
	struct map_session_data *sd;

	if (!script_charid2sd(2,sd))
		return SCRIPT_CMD_FAILURE;

	if(sd) {
		sd->state.night = 1;
		if(map_getmapflag(sd->bl.m,MF_NIGHTENABLED))
			clif_status_load(&sd->bl, EFST_SKE, 1);
	}

	return SCRIPT_CMD_SUCCESS;
}
BUILDIN_FUNC(removenight)
{
	struct map_session_data *sd;

	if (!script_charid2sd(2,sd))
		return SCRIPT_CMD_FAILURE;

	if(sd) {
		sd->state.night = 0;
		clif_status_load(&sd->bl, EFST_SKE, 0);
	}

	return SCRIPT_CMD_SUCCESS;
}
/**
* Generate <ITEML> string for client
* itemlink(<item_id>{,<refine>,<card0>,<card1>,<card2>,<card3>});
* itemlink2(<item_id>,<refine>,<card0>,<card1>,<card2>,<card3>,<RandomIDArray>,<RandomValueArray>,<RandomParamArray>);
* @author [Cydh]
**/
BUILDIN_FUNC(itemlink) {
	unsigned int nameid = script_getnum(st, 2), refine = 0;
	struct s_item_link itemldata;
	memset(&itemldata, 0, sizeof(itemldata));
	FETCH(3, refine);
	FETCH(4, itemldata.cards[0]);
	FETCH(5, itemldata.cards[1]);
	FETCH(6, itemldata.cards[2]);
	FETCH(7, itemldata.cards[3]);
	if (itemldata.cards[0] || itemldata.cards[1] || itemldata.cards[2] || itemldata.cards[3])
		itemldata.flag.cards = 1;
#if PACKETVER >= 20150225
	char *command = (char *)script_getfuncname(st);
	if (command[strlen(command) - 1] == '2') {
		struct item it;
		memset(&it, 0, sizeof(it));
		script_getitem_randomoption(st, NULL, &it, command, 8);
		for (uint8 i = 0; i < MAX_ITEM_RDM_OPT; ++i) {
			if (it.option[i].id)
				itemldata.flag.options = 1;
		}
		memcpy(&itemldata.options, &it.option, sizeof(it.option));
	}
#endif
	std::string itemlstr = createItemLink(nameid, refine, &itemldata);
	char *str = (char *)aMalloc((itemlstr.size() + 1) * sizeof(char));
	safestrncpy(str, itemlstr.c_str(), itemlstr.size() + 1);
	script_pushstr(st, str);
	return SCRIPT_CMD_SUCCESS;
}
#endif
#ifdef M5U00055
BUILDIN_FUNC(geteleresist) {
	struct block_list* bl;
	int type = script_getnum(st,2);

	if(!script_rid2bl(3,bl))
		return SCRIPT_CMD_FAILURE;

	struct map_session_data *sd = map_id2sd(bl->id);
	if(sd)
		script_pushint(st, sd->indexed_bonus.subele_script[type]);
	else
		script_pushint(st, 0);

	return SCRIPT_CMD_SUCCESS;
}
BUILDIN_FUNC(setskillcooldown) {
	struct map_session_data *sd;
	int skillid = script_getnum(st,2);
	int cooldown = script_getnum(st,3);

	if (!script_charid2sd(4,sd))
		return SCRIPT_CMD_FAILURE;
	if (!sd)
		return SCRIPT_CMD_FAILURE;
#ifdef M5B00073
	if (sd->special_state.cooldown_rate)
		cooldown -= cooldown * sd->special_state.cooldown_rate / 100;
#endif

	skill_blockpc_start(sd, skillid, cooldown);

	return SCRIPT_CMD_SUCCESS;
}
BUILDIN_FUNC(isunitvip) {
	struct map_session_data *sd;
	
	if (!script_accid2sd(2,sd))
		return SCRIPT_CMD_FAILURE;

	script_pushint(st, pc_readaccountreg(sd, add_str("#VIP_STATUS")));

	return SCRIPT_CMD_SUCCESS;
}
BUILDIN_FUNC(ispcinit)
{
	struct map_session_data *sd;
	t_tick tick = gettick();

	if (!script_charid2sd(2,sd))
		return SCRIPT_CMD_FAILURE;

	if(sd && (!&sd->sc || !sd->sc.data[SC_INIT]))
		script_pushint(st, 1);
	else
		script_pushint(st, 0);

	return SCRIPT_CMD_SUCCESS;
}
BUILDIN_FUNC(setpcinit)
{
	struct map_session_data *sd;

	if (!script_charid2sd(2,sd))
		return SCRIPT_CMD_FAILURE;

	status_change_end(&sd->bl, SC_INIT, INVALID_TIMER);

	return SCRIPT_CMD_SUCCESS;
}
BUILDIN_FUNC(ispccast)
{
	struct map_session_data *sd;

	if (!script_charid2sd(2,sd))
		return SCRIPT_CMD_FAILURE;

	if(sd->ud.skilltimer != INVALID_TIMER)
		script_pushint(st, 1);
	else
		script_pushint(st, 0);

	return SCRIPT_CMD_SUCCESS;
}
BUILDIN_FUNC(isrental)
{
	struct map_session_data *sd;

	if (!script_charid2sd(2,sd))
		return SCRIPT_CMD_FAILURE;

	int nameid = script_getnum(st, 2);
	int i;

	for (i = 0; i < MAX_INVENTORY; i++) {
		struct item *it = &sd->inventory.u.items_inventory[i];

		if(nameid == it->nameid && it->expire_time) {
			script_pushint(st, 1);
			return SCRIPT_CMD_SUCCESS;
		}
	}
	script_pushint(st, 0);
	return SCRIPT_CMD_SUCCESS;
}
#endif
#ifdef M5U00064
BUILDIN_FUNC(getgdbranchlv)
{
	struct map_session_data* sd = map_id2sd(st->rid);
	int skill_id = ( script_isstring(st, 2) ? skill_name2id(script_getstr(st,2)) : script_getnum(st,2) );

	if (!sd)
		return SCRIPT_CMD_FAILURE;

	struct guild* g = guild_search(sd->status.guild_id);

	if(g)
		script_pushint(st, getgdbranchlv(g, skill_id));
	else
		script_pushint(st, 0);
	return SCRIPT_CMD_SUCCESS;
}
BUILDIN_FUNC(unitstealcoin)
{
	struct map_session_data* sd = map_id2sd(st->rid);
	struct block_list* bl = map_id2bl(script_getnum(st,2));

	if (!sd || !script_rid2bl(2,bl))
		return SCRIPT_CMD_SUCCESS;

	struct mob_data *md = (TBL_MOB*)bl;
	if(md) {
		if(md->state.steal_coin_flag) {
			script_pushint(st, 1);
			return SCRIPT_CMD_SUCCESS;
		}
	}
	script_pushint(st, 0);

	return SCRIPT_CMD_SUCCESS;
}
BUILDIN_FUNC(isitemgroup)
{
	std::shared_ptr<item_data> item_data = item_db.find(script_getnum(st,2));
	std::shared_ptr<s_item_group_db> group = itemdb_group.find(script_getnum(st,3));
	if (!group || !item_data || group->random.empty())
		return SCRIPT_CMD_SUCCESS;
	for (const auto &random : group->random) {
		for (const auto &it : random.second->data) {
			if (it.second->nameid == script_getnum(st,2)) {
				script_pushint(st, 1);
				return SCRIPT_CMD_SUCCESS;
			}
		}
	}
	script_pushint(st, 0);
	return SCRIPT_CMD_SUCCESS;
}
#endif
#ifdef M5U00076
BUILDIN_FUNC(updateinventory)
{
	struct map_session_data* sd = map_id2sd(st->rid);
	if (!sd)
		return SCRIPT_CMD_SUCCESS;
	sd->inventory.max_amount = cap_value(INVENTORY_BASE_SIZE + static_cast<int>(pc_readaccountreg(sd, add_str("#AchievementLevel")))*3 + static_cast<int>(pc_readaccountreg(sd, add_str("#VIP_Type")))*20, 0, MAX_INVENTORY);
	clif_inventory_expansion_info(sd);
	pc_setinventorydata(sd);
	pc_setequipindex(sd);
	pc_check_expiration(sd);
	pc_check_available_item(sd, ITMCHK_INVENTORY);
	pc_itemcd_do(sd, true);
	script_pushint(st, sd->inventory.max_amount);
	return SCRIPT_CMD_SUCCESS;
}
BUILDIN_FUNC(updatestorage)
{
	struct map_session_data* sd = map_id2sd(st->rid);
	int type = script_getnum(st, 2);
	if (!sd)
		return SCRIPT_CMD_SUCCESS;
	if(type == 1)
		sd->storage.max_amount = cap_value(MAX_STORAGE + static_cast<int>(pc_readaccountreg(sd, add_str("#AchievementLevel")))*10, 0, MAX_STORAGE_SQL);
	else
		sd->premiumStorage.max_amount = cap_value(280 + static_cast<int>(pc_readaccountreg(sd, add_str("#VIP_Type")))*80, 0, MAX_STORAGE_SQL);

	return SCRIPT_CMD_SUCCESS;
}
BUILDIN_FUNC(updatecart)
{
	struct map_session_data* sd = map_id2sd(st->rid);

	if (!sd)
		return SCRIPT_CMD_SUCCESS;

	sd->cart.max_amount = cap_value(MAX_CART + static_cast<int>(pc_readaccountreg(sd, add_str("#AchievementLevel")))*3, 0, MAX_CART_SQL);
	if(pc_iscarton(sd)) {
		clif_cartlist(sd);
		clif_updatestatus(sd,SP_CARTINFO);
	}
	return SCRIPT_CMD_SUCCESS;
}
BUILDIN_FUNC(unitbattlemode)
{
	struct block_list* bl;
	enum e_damage_type type = DMG_NODMG2;
	int dmg = 0;
	if(script_hasdata(st,3))
		dmg = script_getnum(st,3);
	if(script_hasdata(st,4))
		type = (e_damage_type)script_getnum(st,4);

	if(!script_rid2bl(2,bl))
		return SCRIPT_CMD_FAILURE;
	//clif_skill_damage(bl,bl,0,0,0,dmg,1,0,0,type);
	clif_damage(bl,bl,0,0,0,dmg,1,type,0,0);

	return SCRIPT_CMD_SUCCESS;
}
BUILDIN_FUNC(whodrop)
{
	struct map_session_data* sd = map_id2sd(st->rid);
	std::shared_ptr<item_data> item_data = item_db.find(script_getnum(st,2));

	if (!sd)
		return SCRIPT_CMD_SUCCESS;
	if (!item_data) {
		script_pushint(st, -1);
		return SCRIPT_CMD_SUCCESS;
	}	

	if (item_data->mob[0].chance == 0)
		script_pushint(st, 0);
	else
		script_pushint(st, 1);
	return SCRIPT_CMD_SUCCESS;
}
#endif
#ifdef M5U00049
BUILDIN_FUNC(battledamage)
{
	struct block_list* src;
	struct block_list* target = map_id2bl(script_getnum(st,3));
	int damage = script_getnum(st,4);
	int ratio = script_getnum(st,5);
	int element = script_getnum(st,6);
	int modes = script_getnum(st,7);
	int options = script_getnum(st,8);
	int heal = 0;
	if(script_hasdata(st,9))
		heal = script_getnum(st,9);

	if(!script_rid2bl(2,src) || !target) {
		script_pushint(st, 0);
		return SCRIPT_CMD_FAILURE;
	}

	struct map_session_data* sd = map_id2sd(src->id);
	struct status_data *sstatus, ///< Attacker status data
		*tstatus; ///< Target status data
	sstatus = status_get_status_data(src);
	tstatus = status_get_status_data(target);
	struct status_change *tsc = status_get_sc(target);
	struct status_change *ssc = status_get_sc(src);

	int min_atk = calc_pc_min_atk(sstatus);
	int max_atk = calc_pc_max_atk(sstatus);
	int rand_atk = calc_pc_rand_atk(sstatus);

	if (modes > 0) {
		if (modes&DMG_ATK) // Damage/heals are based on self's average ATK
			damage += (min_atk + rand()%(rand_atk)) * ratio / 100;
		if (modes&DMG_ATK1) // Damage/heals are based on self's minimum ATK
			damage += min_atk * ratio / 100;
		if (modes&DMG_ATK2) // Damage/heals are based on self's maximum ATK
			damage += max_atk * ratio / 100;
		if (modes&DMG_MATK) // Damage/heals are based on self's average MATK
			damage += (sstatus->matk_min + rand()%(sstatus->matk_max - sstatus->matk_min > 0 ? sstatus->matk_max - sstatus->matk_min : 1)) * ratio / 100;
		if (modes&DMG_MATK1) // Damage/heals are based on self's minimum MATK
			damage += sstatus->matk_min * ratio / 100;
		if (modes&DMG_MATK2) // Damage/heals are based on self's maximum MATK
			damage += sstatus->matk_max * ratio / 100;
		if (modes&DMG_CSP) // Damage/heals are based on self's current SP
			damage += sstatus->sp * ratio / 100;
		if (modes&DMG_LSP) // Damage/heals are based on self's lost SP
			damage += (sstatus->max_sp - sstatus->sp) * ratio / 100;
		if (modes&DMG_CHP) // Damage/heals are based on self's current HP
			damage += sstatus->hp * ratio / 100;
		if (modes&DMG_LHP) // Damage/heals are based on self's lost HP
			damage += (sstatus->max_hp - sstatus->hp) * ratio / 100;

		if (modes&DMG_DEF) // Damage/heals are based on self's DEF
			damage += sstatus->def * damage / 100;
		if (modes&DMG_MDEF) // Damage/heals are based on self's MDEF
			damage += sstatus->mdef * damage / 100;
		if (modes&DMG_CRI) // Damage/heals are based on self's CRI
			damage += sstatus->cri * damage / 1000;
		if (modes&DMG_STR) // Damage/heals are based on self's STR
			damage += sstatus->str * damage / 100;
		if (modes&DMG_DEX) // Damage/heals are based on self's DEX
			damage += sstatus->dex * damage / 100;
		if (modes&DMG_INT) // Damage/heals are based on self's INT
			damage += sstatus->int_ * damage / 100;
		if (modes&DMG_VIT) // Damage/heals are based on self's VIT
			damage += sstatus->vit * damage / 100;
		if (modes&DMG_AGI) // Damage/heals are based on self's AGI
			damage += sstatus->agi * damage / 100;
		if (modes&DMG_LUK) // Damage/heals are based on self's LUK
			damage += sstatus->luk * damage / 100;

		if (modes&DMG_HPLEFT) // Damage/heals are based on self's current HP (the higher, the better)
			damage = damage * (100 + (sstatus->hp * 100 / (sstatus->max_hp > 0 ? sstatus->max_hp : 1))) / 100;
		if (modes&DMG_HPLOST) // Damage/heals are based on self's current HP (the lower, the better)
			damage = damage * (100 + (100 - sstatus->hp * 100 / (sstatus->max_hp > 0 ? sstatus->max_hp : 1))) / 100;
		if (modes&DMG_SPLEFT) // Damage/heals are based on self's current SP (the higher, the better)
			damage = damage * (100 + (sstatus->sp * 100 / (sstatus->max_sp > 0 ? sstatus->max_sp : 1))) / 100;
		if (modes&DMG_SPLOST) // Damage/heals are based on self's current SP (the lower, the better)
			damage = damage * (100 + (100 - sstatus->sp * 100 / (sstatus->max_sp > 0 ? sstatus->max_sp : 1))) / 100;
	}

	// Options
	if (options&DEF_DEF) // Damage/heals are reduced by target's DEF
		damage = damage * (100 - tstatus->def) / 100;
	if (options&DEF_MDEF) // Damage/heals are reduced by target's MDEF
		damage = damage * (100 - tstatus->mdef) / 100;
	if (options&DEF_STR) // Damage/heals are reduced by target's STR
		damage = damage * 100 / (100 + tstatus->str);
	if (options&DEF_DEX) // Damage/heals are reduced by target's DEX
		damage = damage * 100 / (100 + tstatus->dex);
	if (options&DEF_INT) // Damage/heals are reduced by target's INT
		damage = damage * 100 / (100 + tstatus->int_);
	if (options&DEF_VIT) // Damage/heals are reduced by target's VIT
		damage = damage * 100 / (100 + tstatus->vit);
	if (options&DEF_AGI) // Damage/heals are reduced by target's AGI
		damage = damage * 100 / (100 + tstatus->agi);
	if (options&DEF_LUK) // Damage/heals are reduced by target's LUK
		damage = damage * 100 / (100 + tstatus->luk);
	if (options&DEF_RDEF) // Damage/heals are increased by target's DEF
		damage = damage * (100 + tstatus->def) / 100;
	if (options&DEF_RMDEF) // Damage/heals are increased by target's MDEF
		damage = damage * (100 + tstatus->mdef) / 100;
	if (options&DEF_RSTR) // Damage/heals are increased by target's STR
		damage = damage * (100 + tstatus->str / 2) / 100;
	if (options&DEF_RDEX) // Damage/heals are increased by target's DEX
		damage = damage * (100 + tstatus->dex / 2) / 100;
	if (options&DEF_RINT) // Damage/heals are increased by target's INT
		damage = damage * (100 + tstatus->int_ / 2) / 100;
	if (options&DEF_RVIT) // Damage/heals are increased by target's VIT
		damage = damage * (100 + tstatus->vit / 2) / 100;
	if (options&DEF_RAGI) // Damage/heals are increased by target's AGI
		damage = damage * (100 + tstatus->agi / 2) / 100;
	if (options&DEF_RLUK) // Damage/heals are increased by target's LUK
		damage = damage * (100 + tstatus->luk / 2) / 100;
	if (options&DEF_SPLEFT && target->type == BL_PC) // Damage/heals are increased by target's current SP ratio
		damage = damage * (100 + (tstatus->sp * 100 / (tstatus->max_sp > 0 ? tstatus->max_sp : 1))) / 100;
	if (options&DEF_SPLOST && target->type == BL_PC) // Damage/heals are increased by target's current HP (the lower, the better)
		damage = damage * (100 + (100 - tstatus->sp * 100 / (tstatus->max_sp > 0 ? tstatus->max_sp : 1))) / 100;
	if (options&DEF_HPLEFT) // Damage/heals are increased by target's current HP (the higher, the better)
		damage = damage * (100 + (tstatus->hp * 100 / (tstatus->max_hp > 0 ? tstatus->max_hp : 1))) / 100;
	if (options&DEF_HPLOST) // Damage/heals are increased by target's current HP (the lower, the better)
		damage = damage * (100 + (100 - tstatus->hp * 100 / (tstatus->max_hp > 0 ? tstatus->max_hp : 1))) / 100;

	// Each modifiers increase damage by 50%
	int modifier = 0;
	if (options&DEF_MOB && target->type == BL_MOB)
		modifier += 1;
	if (options&DEF_BOSS && target->type == BL_MOB && status_bl_has_mode(target,MD_STATUSIMMUNE))
		modifier += 1;
	if (options&DEF_MVP && target->type == BL_MOB && status_bl_has_mode(target,MD_MVP))
		modifier += 1;
	if (options&DEF_FLEE)
		modifier += 1;

	// DEF_MODX1 add 50% for each modifiers and DEF_MODX2 add 100% for each modifiers, then are cumulative in order to add 150% per modifiers
	if (modifier > 0)
		damage += modifier * (1 + (options&DEF_MODX1 ? 1 : 0) + (options&DEF_MODX2 ? 2 : 0)) * damage / 2;

	if (element != ELE_NONE && (element = (element == ELE_WEAPON) ? (sd->bonus.arrow_ele ? sd->bonus.arrow_ele : sstatus->rhw.ele) : element))
		damage = (int) battle_attr_fix(src, target, (int64) damage, element, tstatus->def_ele, tstatus->ele_lv);

	struct map_data *mapdata = map_getmapdata(target->m);
	if( mapdata_flag_gvg2(mapdata) )
		damage = damage * 60 / 100;

	if(heal) {
		if(!(options&DEF_SP))
			add_timer(gettick()+50,status_heal_hp_timer,target->id,damage);
		else
			add_timer(gettick()+50,status_heal_sp_timer,target->id,damage);
	} else {
		status_damage(src,target,damage,0,0,0,0);
		clif_damage(target, target, gettick(), 500, 100, damage, 1, DMG_ENDURE, damage, true);
	}

	script_pushint(st, damage);
	return SCRIPT_CMD_SUCCESS;
}
BUILDIN_FUNC(unitskilleffect)
{
	struct block_list* src;
	struct block_list* target = map_id2bl(script_getnum(st,3));

	if(!script_rid2bl(2,src) || !target)
		return SCRIPT_CMD_FAILURE;

	uint16 skill_id, skill_lv;
	skill_id = ( script_isstring(st, 4) ? skill_name2id(script_getstr(st,4)) : script_getnum(st,4) );
	skill_lv = script_getnum(st,5);
	int duration = script_getnum(st,6);

	clif_skill_nodamage(src,target,skill_id,skill_lv,duration);
	return SCRIPT_CMD_SUCCESS;
}
BUILDIN_FUNC(clearnpc)
{
	struct map_session_data* sd = map_id2sd(st->rid);
	if(!sd)
	return SCRIPT_CMD_SUCCESS;

	clif_clearunit_single(sd->npc_id, CLR_OUTSIGHT, sd->fd);

	return SCRIPT_CMD_SUCCESS;
}
BUILDIN_FUNC(getskillammo)
{
	int skill_id = ( script_isstring(st, 2) ? skill_name2id(script_getstr(st,2)) : script_getnum(st,2) );
	uint16 idx = 0;

	if ((idx = skill_get_index(skill_id)) == 0)
		return SCRIPT_CMD_FAILURE;

	script_pushint(st, skill_get_ammotype(skill_id));
	return SCRIPT_CMD_SUCCESS;
}
BUILDIN_FUNC(getskillammoqty)
{
	int skill_id = ( script_isstring(st, 2) ? skill_name2id(script_getstr(st,2)) : script_getnum(st,2) );
	int skill_lv = script_getnum(st, 3);
	uint16 idx = 0;

	if ((idx = skill_get_index(skill_id)) == 0)
		return SCRIPT_CMD_FAILURE;

	script_pushint(st, skill_get_ammo_qty(skill_id,skill_lv));
	return SCRIPT_CMD_SUCCESS;
}
BUILDIN_FUNC(inventoryselect2)
{
	struct map_session_data* sd;
	int type = script_getnum(st,2);

	if (!script_rid2sd(sd))
		return SCRIPT_CMD_SUCCESS;

#ifdef M5U00063
	clif_sendmisc(sd,type);
#endif

	return SCRIPT_CMD_SUCCESS;
}
BUILDIN_FUNC(gethand)
{
	struct map_session_data* sd;

	if (!script_rid2sd(sd))
		return SCRIPT_CMD_SUCCESS;

	if(sd->state.lr_flag)
		script_pushint(st, 1);
	else
		script_pushint(st, 0);

	return SCRIPT_CMD_SUCCESS;
}
#endif
#ifdef M5U00142
BUILDIN_FUNC(getmadoinfo)
{
	struct map_session_data* sd;
	int type = script_getnum(st, 2);

	if (!script_rid2sd(sd))
		return SCRIPT_CMD_SUCCESS;
	if(type == 0)
		script_pushint(st, sd->hp_mado);
	else if(type == 1)
		script_pushint(st, sd->max_hp_mado);

	return SCRIPT_CMD_SUCCESS;
}
BUILDIN_FUNC(setmadoinfo)
{
	struct map_session_data* sd;
	int type = script_getnum(st, 2);
	int val = script_getnum(st, 3);

	if (!script_rid2sd(sd))
		return SCRIPT_CMD_SUCCESS;
	if(type == 0)
		sd->hp_mado = val;
	else if(type == 1)
		sd->max_hp_mado = val;
	clif_updatestatus(sd,SP_HP);
	clif_updatestatus(sd,SP_MAXHP);

	return SCRIPT_CMD_SUCCESS;
}
#endif
#ifdef M5U00090
BUILDIN_FUNC(displaydmg)
{
	struct block_list* target;
	struct block_list* src = map_id2bl(script_getnum(st,3));
	int dmg = script_getnum(st,4);

	if(!script_rid2bl(2,target) || !src)
		return SCRIPT_CMD_FAILURE;

	clif_damage(src, target, gettick(), 0, 0, dmg, 1, DMG_NORMAL, 0, 0);
	return SCRIPT_CMD_SUCCESS;
}
BUILDIN_FUNC(getmountinfo)
{
	struct map_session_data* sd;
	int type = script_getnum(st, 2);

	if (!script_rid2sd(sd))
		return SCRIPT_CMD_SUCCESS;
	if(type == MOUNT_HP)
		script_pushint(st, sd->hp_mount);
	else if(type == MOUNT_SP)
		script_pushint(st, sd->sp_mount);
	else if(type == MOUNT_MAXHP)
		script_pushint(st, sd->max_hp_mount);
	else if(type == MOUNT_MAXSP)
		script_pushint(st, sd->max_sp_mount);

	return SCRIPT_CMD_SUCCESS;
}
BUILDIN_FUNC(setmountinfo)
{
	struct map_session_data* sd;
	int type = script_getnum(st, 2);
	int val = script_getnum(st, 3);

	if (!script_rid2sd(sd))
		return SCRIPT_CMD_SUCCESS;
	if(type == MOUNT_HP)
		sd->hp_mount = val;
	else if(type == MOUNT_SP)
		sd->sp_mount = val;
	else if(type == MOUNT_MAXHP)
		sd->max_hp_mount = val;
	else if(type == MOUNT_MAXSP)
		sd->max_sp_mount = val;
	clif_updatestatus(sd,SP_HP);
	clif_updatestatus(sd,SP_SP);
	clif_updatestatus(sd,SP_MAXHP);
	clif_updatestatus(sd,SP_MAXSP);
	//clif_hpmeter(sd);

	return SCRIPT_CMD_SUCCESS;
}
BUILDIN_FUNC(getmapid)
{
	const char *mapname;
	int16 m;

	mapname = script_getstr(st, 2);
	m = map_mapname2mapid(mapname);

	script_pushint(st, m);

	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(setpetcalc)
{
	struct map_session_data* sd;
	int type = script_getnum(st, 2);

	if (!script_rid2sd(sd))
		return SCRIPT_CMD_SUCCESS;
	sd->petnocalc = type;
	return SCRIPT_CMD_SUCCESS;
}
BUILDIN_FUNC(addstorageitem)
{
	struct map_session_data* sd;
	int item_id = script_getnum(st, 2);

	if (!script_rid2sd(sd))
		return SCRIPT_CMD_SUCCESS;

	for (int i = 0; i < MAX_INVENTORY; i++)
		if (sd->inventory.u.items_inventory[i].nameid == item_id)
			storage_storageadd(sd, &sd->storage, i, sd->inventory.u.items_inventory[i].amount, 1);

	return SCRIPT_CMD_SUCCESS;
}
BUILDIN_FUNC(shopresult)
{
	struct map_session_data* sd;

	if (!script_rid2sd(sd))
		return SCRIPT_CMD_SUCCESS;
	e_purchase_result result;
	switch(script_getnum(st,2)) {
		case 11: result = e_purchase_result::PURCHASE_FAIL_EXCHANGE_FAILED; break;
	}
	clif_npc_buy_result(sd, result);
	return SCRIPT_CMD_SUCCESS;
}
BUILDIN_FUNC(getqueuecount)
{
	struct map_session_data* sd;
	const char *name = script_getstr(st, 2);

	if (!script_rid2sd(sd))
		return SCRIPT_CMD_SUCCESS;

	int16 mapindex = mapindex_name2id(name);

	for (auto &pair : battleground_db) {
		for (auto &map : pair.second->maps) {
			if (map.mapindex == mapindex) {
				for (const auto &queue : bg_queues) {
					if(pair.second->id == queue->id) {
						script_pushint(st, queue->teama_members.size()+queue->teamb_members.size());
						return SCRIPT_CMD_SUCCESS;
					}
				}
				script_pushint(st, 0);
				return SCRIPT_CMD_SUCCESS;
			}
		}
	}
	script_pushint(st, 0);
	return SCRIPT_CMD_SUCCESS;
}
BUILDIN_FUNC(plagia)
{
	struct map_session_data* sd;
	int skill_id = script_getnum(st, 2);

	if (!script_rid2sd(sd))
		return SCRIPT_CMD_SUCCESS;

	std::shared_ptr<s_skill_db> skill = skill_db.find(skill_id);
	if(skill && skill->copyable.option & SKILL_COPY_PLAGIARISM) {
		skill_do_copy(&sd->bl,&sd->bl,skill_id,skill->max);
		script_pushint(st, 1);
	} else
		script_pushint(st, 0);
	return SCRIPT_CMD_SUCCESS;
}
BUILDIN_FUNC(getcharm)
{
	struct map_session_data* sd;
	int charm = script_getnum(st, 2);

	if (!script_rid2sd(sd))
		return SCRIPT_CMD_SUCCESS;
	switch(charm) {
		case 0: default: script_pushint(st, sd->spiritcharm); break;
		case 1: script_pushint(st, sd->spiritcharm_type); break;
		case 2: script_pushint(st, sd->soulball); break;
	}
	return SCRIPT_CMD_SUCCESS;
}
BUILDIN_FUNC(getspirit)
{
	struct map_session_data* sd;
	int charm = script_getnum(st, 2);

	if (!script_rid2sd(sd))
		return SCRIPT_CMD_SUCCESS;
	switch(charm) {
		case 0: default: script_pushint(st, sd->spiritcharm); break;
		case 1: script_pushint(st, sd->spiritcharm_type); break;
	}
	return SCRIPT_CMD_SUCCESS;
}
BUILDIN_FUNC(getequiprental)
{
	int i, num;
	TBL_PC* sd;
	struct item* item;

	if (!script_charid2sd(3, sd)) {
		script_pushconststr(st, "");
		return SCRIPT_CMD_FAILURE;
	}

	num = script_getnum(st,2);
	if ( !equip_index_check(num) ) {
		script_pushconststr(st, "");
		return SCRIPT_CMD_FAILURE;
	}

	// get inventory position of item
	i = pc_checkequip(sd,equip_bitmask[num]);
	if (i < 0) {
		script_pushconststr(st, "");
		return SCRIPT_CMD_FAILURE;
	}

	item = &sd->inventory.u.items_inventory[i];

	if(item->expire_time || item->bound)
		script_pushint(st, 1);
	else
		script_pushint(st, 0);
	return SCRIPT_CMD_SUCCESS;
}
#endif
#ifdef M5U00119
BUILDIN_FUNC(settraining)
{
	struct map_session_data* sd;

	if (!script_rid2sd(sd))
		return SCRIPT_CMD_SUCCESS;
	sd->trainingroom = 1;
	return SCRIPT_CMD_SUCCESS;
}
BUILDIN_FUNC(gettraining)
{
	struct block_list* src;
	if(script_hasdata(st,2))
		src = map_id2bl(script_getnum(st,2));
	else
		src = map_id2bl(st->rid);

	if(!src)
		return SCRIPT_CMD_FAILURE;

	struct map_session_data* sd = map_id2sd(src->id);
	if(sd && sd->trainingroom)
		script_pushint(st, 1);
	else
		script_pushint(st, 0);
	return SCRIPT_CMD_SUCCESS;
}
#endif

#ifdef M5U00126
static int clearunit_sub(struct block_list *bl,va_list ap)
{
	nullpo_ret(bl);
	skill_clear_unitgroup(bl);
	return SCRIPT_CMD_SUCCESS;
}
BUILDIN_FUNC(clearskillunit)
{
	const char *map_name = script_getstr(st, 2);
	int m = map_mapname2mapid(map_name);

	if(script_hasdata(st,3)) {
		struct block_list *bl = map_id2bl(script_getnum(st,3));
		if(bl)
			skill_clear_unitgroup(bl);
		return SCRIPT_CMD_SUCCESS;
	}

	map_foreachinmap(clearunit_sub, m, BL_PC);
	return SCRIPT_CMD_SUCCESS;
}
#endif
#ifdef M5U00126
BUILDIN_FUNC(nochecknear)
{
	struct map_session_data* sd;
	int type = script_getnum(st,2);

	if (!script_rid2sd(sd))
		return SCRIPT_CMD_SUCCESS;

	sd->state.no_checknear = type;
	return SCRIPT_CMD_SUCCESS;
}
#endif
#ifdef M5U00127
BUILDIN_FUNC(autospell)
{
	struct map_session_data *sd = NULL;
	struct block_list* src;
	struct block_list* bl = map_id2bl(script_getnum(st,3));
	int skill_id = ( script_isstring(st, 4) ? skill_name2id(script_getstr(st,4)) : script_getnum(st,4) );
	int skill_lv = script_getnum(st,5);
	int type = script_getnum(st,6);
	int delay = script_hasdata(st,8) ? script_getnum(st,8) : 0;
	src = map_id2bl(script_hasdata(st,7) ? script_getnum(st,7) : script_getnum(st,2));

	if(!bl || !src)
		return SCRIPT_CMD_SUCCESS;
	if(src->type == BL_PC) {
		sd = (TBL_PC*)src;
	}
	if(sd) {
		sd->state.autocast = 1;
		if ( skill_isNotOk(skill_id, sd) )
			return SCRIPT_CMD_SUCCESS;
	}
	if(skill_lv < 1)
		skill_lv = 1;

	if (type == 3 && !skill_pos_maxcount_check(src, bl->x, bl->y, skill_id, skill_lv, BL_PC, false))
		return SCRIPT_CMD_SUCCESS;

	if (!battle_check_range(src, bl, skill_get_range2(src, skill_id, skill_lv, true)))
		return SCRIPT_CMD_SUCCESS;

	switch(type) {
		case 1: skill_castend_damage_id(src, bl, skill_id, skill_lv, delay, 0); break;
		case 2: skill_castend_nodamage_id(src, bl, skill_id, skill_lv, delay, 0); break;
		case 3: skill_castend_pos2(src, bl->x, bl->y, skill_id, skill_lv, delay, 0); break;
	}
	if(sd)
		sd->state.autocast = 0;

	return SCRIPT_CMD_SUCCESS;
}
#endif
#ifdef M5U00136
BUILDIN_FUNC(getcurrentid)
{
	struct map_session_data* sd;

	if (!script_rid2sd(sd))
		return SCRIPT_CMD_SUCCESS;

	script_pushint(st, sd->itemid);
	return SCRIPT_CMD_SUCCESS;
}
BUILDIN_FUNC(getcurrentpos)
{
	struct map_session_data* sd;

	if (!script_rid2sd(sd))
		return SCRIPT_CMD_SUCCESS;

	script_pushint(st, sd->itemindex);
	return SCRIPT_CMD_SUCCESS;
}
BUILDIN_FUNC(getequipweighttype)
{
	struct map_session_data* sd;
	int type = script_getnum(st,2);

	if (!script_rid2sd(sd))
		return SCRIPT_CMD_SUCCESS;
	switch(type) {
		case EQI_ARMOR: script_pushint(st, sd->armortype); break;
		case EQI_GARMENT: script_pushint(st, sd->garmenttype); break;
		case EQI_SHOES: script_pushint(st, sd->shoestype); break;
		case EQI_HAND_L: script_pushint(st, sd->shieldtype); break;
	}
	return SCRIPT_CMD_SUCCESS;
}
BUILDIN_FUNC(setequipweighttype)
{
	struct map_session_data* sd;
	int type = script_getnum(st,2);

	if (!script_rid2sd(sd))
		return SCRIPT_CMD_SUCCESS;
	switch(type) {
		case EQI_ARMOR: sd->armortype = script_getnum(st,3); break;
		case EQI_GARMENT: sd->garmenttype = script_getnum(st,3); break;
		case EQI_SHOES: sd->shoestype = script_getnum(st,3); break;
		case EQI_HAND_L: sd->shieldtype = script_getnum(st,3); break;
	}
	return SCRIPT_CMD_SUCCESS;
}
BUILDIN_FUNC(setachievementgrade)
{
	struct map_session_data* sd;
	int value = script_getnum(st,2);

	if (!script_rid2sd(sd))
		return SCRIPT_CMD_SUCCESS;
	sd->achievementgrade = value;
	return SCRIPT_CMD_SUCCESS;
}
BUILDIN_FUNC(capture)
{
	struct map_session_data* sd;

	if (!script_rid2sd(sd))
		return SCRIPT_CMD_SUCCESS;

	clif_catch_process(sd);
	return SCRIPT_CMD_SUCCESS;
}
BUILDIN_FUNC(setpconcart)
{
	struct map_session_data* sd;
	int id = script_getnum(st,2);

	if (!script_rid2sd(sd))
		return SCRIPT_CMD_SUCCESS;

	sd->oncart = id;
	return SCRIPT_CMD_SUCCESS;
}
BUILDIN_FUNC(getpconcart)
{
	struct map_session_data* sd;
	int id = script_getnum(st,2);

	if (!script_rid2sd(sd))
		return SCRIPT_CMD_SUCCESS;

	script_pushint(st, sd->oncart);
	return SCRIPT_CMD_SUCCESS;
}
BUILDIN_FUNC(clearnpcmenu)
{
	struct map_session_data* sd;

	if (!script_rid2sd(sd))
		return SCRIPT_CMD_SUCCESS;

	sd->npc_menu = 0;
	return SCRIPT_CMD_SUCCESS;
}
BUILDIN_FUNC(gettombinfo) {
	TBL_NPC *nd = map_id2nd(script_getnum(st,2));
	int info = script_getnum(st,3);
	if(!nd)
		return SCRIPT_CMD_FAILURE;
	if(nd->subtype == NPCTYPE_TOMB) {
		switch(info) {
			case 0: script_pushint(st, nd->u.tomb.md->bl.id); return SCRIPT_CMD_SUCCESS;
			case 1: script_pushint(st, nd->u.tomb.md->mob_id); return SCRIPT_CMD_SUCCESS;
			case 2: script_pushint(st, nd->u.tomb.kill_time); return SCRIPT_CMD_SUCCESS;
			case 3: script_pushint(st, nd->u.tomb.spawn_timer); return SCRIPT_CMD_SUCCESS;
			case 4: script_pushstrcopy(st, nd->u.tomb.killer_name); return SCRIPT_CMD_SUCCESS;
		}
	}
	script_pushint(st, 0);
	return SCRIPT_CMD_SUCCESS;
}
BUILDIN_FUNC(setviewmvp)
{
	struct map_session_data* sd;

	if( !script_hasdata(st, 2) )
		script_rid2sd(sd);
	else
		sd = map_id2sd( script_getnum(st, 2) );

	if(sd) {
		struct mob_data *md = map_id2md(script_getnum(st, 3));
		if(md) {
			md->state.boss = 1;
			clif_bossmapinfo(sd, md, (md->spawn_timer != INVALID_TIMER) ? BOSS_INFO_DEAD : BOSS_INFO_ALIVE_WITHMSG);
		}
	}

	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(guildskillreset)
{
	struct map_session_data* sd;
	struct guild* guild_data;
	int guild_id;

	if (!script_rid2sd(sd)) {
		script_pushint(st, false);
		return SCRIPT_CMD_FAILURE;
	}

	if (script_hasdata(st, 2))
		guild_id = script_getnum(st, 2);
	else
		guild_id = sd->status.guild_id;

	guild_data = guild_search(guild_id);
	struct guild* g = sd->guild;
	if (g && guild_data && guild_data->member[0].char_id == sd->status.char_id) {
		if( SQL_SUCCESS != Sql_Query(mmysql_handle, "DELETE FROM `%s` WHERE `guild_id` = '%d'", guild_skill_table, sd->status.guild_id) )
			return SCRIPT_CMD_SUCCESS;
		g->skill_point = g->guild_lv-1;
		for(int i = 0; i < MAX_GUILDSKILL; i++)
			g->skill[i].lv = 0;
	} else
		script_pushint(st, false);

	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(getpartycount)
{
	struct map_session_data* sd;
	struct party_data* p;
	int type = script_getnum(st,2);
	int count = 0, i = 0;

	if (!script_rid2sd(sd)) {
		script_pushint(st, 1);
		return SCRIPT_CMD_FAILURE;
	}

	if(!(p = party_search(sd->status.party_id))) {
		if(type == 3)
			setd_sub_num( st, NULL, ".@memberid", 0, sd->status.account_id, NULL );
		script_pushint(st, 1);
		return SCRIPT_CMD_SUCCESS;
	}

	switch(type) {
		case 0: // Players Party Size
			ARR_FIND(0, MAX_PARTY, i, p->party.member[i].account_id == 0);
			script_pushint(st, i);
			break;
		case 1: // Online players
			for(i = 0; i < MAX_PARTY; i++) {
				if(p->party.member[i].online)
					count++;
			}
			script_pushint(st, count);
			break;
		case 2: // Online players on the same map
			script_pushint(st, p->party.count); break;
			break;
		case 3: // Get online players account ID on the same map
			for(i = 0; i < MAX_PARTY; i++) {
				if(p->data[i].sd && p->data[i].sd->mapindex == sd->mapindex) {
					setd_sub_num( st, NULL, ".@memberid", count, p->data[i].sd->status.account_id, NULL );
					count++;
				}
			}
			script_pushint(st, count);
			break;
	}

	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(getbonus)
{
	struct map_session_data* sd;
	int type = script_getnum(st,2);

	if (!script_rid2sd(sd)) {
		script_pushint(st, 1);
		return SCRIPT_CMD_FAILURE;
	}
	switch(type) {
		default: script_pushint(st, 0); break;
		case 0: script_pushint(st, sd->special_state.no_spell_scroll_use_rate); break;
	}
	return SCRIPT_CMD_SUCCESS;
}

/**
* makeitem <item id>,<amount>,"<map name>",<X>,<Y>,<rangex>,<rangey>;
* makeitem "<item name>",<amount>,"<map name>",<X>,<Y>,<rangex>,<rangey>;
*/
BUILDIN_FUNC(makeitemarea) {
	uint16 nameid, amount, flag = 0;
	int16 x = 0, y = 0, rx = 0, ry = 0;
	const char *mapname;
	int m;
	struct item item_tmp;

	if( script_isstring(st, 2) ){
		const char *name = script_getstr(st, 2);
		std::shared_ptr<item_data> item_data = item_db.searchname(name);

		if( item_data )
			nameid = item_data->nameid;
		else
			nameid = UNKNOWN_ITEM_ID;
	}
	else
		nameid = script_getnum(st, 2);

	amount = script_getnum(st,3);
	mapname	= script_getstr(st,4);
	x = script_getnum(st,5);
	y = script_getnum(st,6);
	rx = script_getnum(st,7);
	ry = script_getnum(st,8);
#ifdef M5U00104
	int pickup = 0;
	if(script_hasdata(st,9))
		pickup = script_getnum(st,9);
#endif

	if(strcmp(mapname,"this")==0) {
		TBL_PC *sd;
		if (!script_rid2sd(sd))
			return SCRIPT_CMD_SUCCESS; //Failed...
		m = sd->bl.m;
	} else
		m = map_mapname2mapid(mapname);

	if(nameid<0) {
		nameid = -nameid;
		flag = 1;
	}

	if(nameid > 0) {
		memset(&item_tmp,0,sizeof(item_tmp));
		item_tmp.nameid = nameid;
		if(!flag)
			item_tmp.identify = 1;
		else
			item_tmp.identify = itemdb_isidentified(nameid);

		struct block_list* bl = map_id2bl(st->rid);
		map_search_freecell(bl, m, &x, &y, rx, ry, 1);
#ifdef M5U00104
		map_addflooritem(&item_tmp,amount,m,x,y,pickup,pickup,pickup,4,0,true);
#else
		map_addflooritem(&item_tmp,amount,m,x,y,0,0,0,4,0);
#endif
	}
	return SCRIPT_CMD_SUCCESS;
}
#endif

BUILDIN_FUNC(makepet2)
{
	struct map_session_data* sd;
	std::shared_ptr<s_pet_db> pet = pet_db_search(script_getnum(st,2), PET_EGG);

	if( !script_rid2sd(sd) || !pet )
		return SCRIPT_CMD_FAILURE;

	std::shared_ptr<s_mob_db> mdb = mob_db.find(pet->class_);

	if( mdb == nullptr ){
		return false;
	}
	sd->catch_target_class = pet->class_;
	intif_create_pet(sd->status.account_id, sd->status.char_id, pet->class_, mdb->lv, pet->EggID, 0, pet->intimate, 100, 0, 1, mdb->jname.c_str());

	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(getachievementlevel) {
	struct map_session_data *sd;

	if (!script_charid2sd(2, sd)) {
		script_pushint(st, false);
		return SCRIPT_CMD_FAILURE;
	}

	script_pushint(st, sd->achievement_data.level);

	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(party_getoption)
{
	struct party_data *party;
	int type = script_getnum(st,3);

	if( !(party = party_search(script_getnum(st,2))) ) {
		script_pushint(st,1);
		return SCRIPT_CMD_SUCCESS;
	}

	script_pushint(st,party->party.exp);

	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(mapname2id)
{
	script_pushint(st, map_mapname2mapid(script_getstr(st, 2)));

	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(getvar3) {
	int char_id = script_getnum(st, 4);
	int i = script_getnum(st, 3);
	const char* var = script_getstr(st, 2);
	struct map_session_data *sd = map_charid2sd(char_id);
	if (!sd) {
		return SCRIPT_CMD_FAILURE;
	}
	if(script_hasdata(st, 5)) {
		struct script_data* data = script_getdata(st, 5);
		script_pushstr(st, pc_readregstr(sd,reference_uid(add_str(var), i)));
	} else
		script_pushint(st, pc_readreg(sd,reference_uid(add_str(var), i)));
	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(setvar2) {
	int char_id = script_getnum(st, 5);
	int i = script_getnum(st, 3);
	const char* var = script_getstr(st, 2);
	struct map_session_data *sd = map_charid2sd(char_id);
	if (!sd) {
		return SCRIPT_CMD_FAILURE;
	}
	struct script_data* data = script_getdata(st, 4);
	if(data_isstring(data))
		script_pushint(st, pc_setregstr(sd,reference_uid(add_str(var), i),script_getstr(st, 4)));
	else
		script_pushint(st, pc_setreg(sd,reference_uid(add_str(var), i), script_getnum(st, 4)));
	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(setmap) {
	struct map_session_data *sd;
	int i = script_getnum(st, 2);

	if (!script_rid2sd(sd)) {
		script_pushint(st, -1);
		return SCRIPT_CMD_FAILURE;
	}

	sd->currentmap = i;
	script_pushint(st, sd->currentmap);

	return SCRIPT_CMD_SUCCESS;
}


BUILDIN_FUNC(getbattleinfo) {
	struct map_session_data* sd = map_id2sd(st->rid);
	int i = script_getnum(st, 2);
	if(script_hasdata(st,3))
		sd = map_id2sd(script_getnum(st,3));
	if(!sd)
		return SCRIPT_CMD_SUCCESS;
	switch(i) {
		default: script_pushint(st, -1); break;
		case BATTLE_ISCRIT: script_pushint(st, sd->battle_info.iscrit); break;
		case BATTLE_ISDIST: script_pushint(st, sd->battle_info.isdist); break;
		case BATTLE_DAMAGE: script_pushint(st, sd->battle_info.damage); break;
		case BATTLE_SKILLID: script_pushint(st, sd->battle_info.skill_id); break;
		case BATTLE_ELE: script_pushint(st, sd->battle_info.element); break;
		case BATTLE_DEFELE: script_pushint(st, sd->battle_info.defele); break;
		case BATTLE_DEFRACE: script_pushint(st, sd->battle_info.defrace); break;
		case BATTLE_DEFSIZE: script_pushint(st, sd->battle_info.defsize); break;
		case BATTLE_SRCID: script_pushint(st, sd->battle_info.srcid); break;
	}
	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(setenchantcharge) {
	struct map_session_data* sd = map_id2sd(st->rid);
	int i = script_getnum(st, 2);
	if(script_hasdata(st,3))
		sd = map_id2sd(script_getnum(st,3));
	if(!sd)
		return SCRIPT_CMD_SUCCESS;
	script_pushint(st, sd->enchantcharge);
	return SCRIPT_CMD_SUCCESS;
}
BUILDIN_FUNC(getenchantcharge) {
	struct map_session_data* sd = map_id2sd(st->rid);
	if(script_hasdata(st,2))
		sd = map_id2sd(script_getnum(st,2));
	if(!sd)
		return SCRIPT_CMD_SUCCESS;
	script_pushint(st, sd->enchantcharge);
	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(setguild) {
	struct map_session_data* sd = map_id2sd(st->rid);
	int guild_id = script_getnum(st, 2);
	if(script_hasdata(st,3))
		sd = map_id2sd(script_getnum(st,3));
	if(!sd)
		return SCRIPT_CMD_SUCCESS;
	struct guild_member m;
	struct guild* g = guild_search(guild_id);
	int i = 0;
	sd->guild_invite = 1;
	sd->guild_invite_account = g->member[0].char_id;

	if( g == NULL ) {
		sd->guild_invite = 0;
		sd->guild_invite_account = 0;
		return 0;
	}
	ARR_FIND( 0, g->max_member, i, g->member[i].account_id == 0 );
	if( i == g->max_member ) {
		sd->guild_invite = 0;
		sd->guild_invite_account = 0;
		return 0;
	}

	guild_makemember(&m,sd);
	intif_guild_addmember(guild_id, &m);
	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(recalcequiptype) {
	struct map_session_data* sd = map_id2sd(st->rid);
	if(script_hasdata(st,2))
		sd = map_id2sd(script_getnum(st,2));
	if(!sd)
		return SCRIPT_CMD_SUCCESS;
#ifdef M5U00056
	int i = 0, k = 0;
	struct item_data *id;
	int num[4] = { EQI_HAND_L, EQI_ARMOR, EQI_SHOES, EQI_GARMENT };
	for (i = 0; i < 4; i++) {
		if (equip_index_check(num[i]))
			k = pc_checkequip(sd, equip_bitmask[num[i]]);
		if (k >= 0 && k < MAX_INVENTORY && sd->inventory_data[k]) {
			id = sd->inventory_data[k];
			switch (num[i]) {
				case EQI_HAND_L:
					sd->shieldtype = id->def < 2 ? EQI_LIGHT : id->def < 3 ? EQI_MEDIUM : id->def < 4 ? EQI_NEUTRAL : id->def < 6 ? EQI_HEAVY : EQI_VHEAVY;
					break;
				case EQI_ARMOR:
					sd->armortype = id->def < 4 ? EQI_LIGHT : id->def < 6 ? EQI_MEDIUM : id->def < 8 ? EQI_NEUTRAL : id->def < 10 ? EQI_HEAVY : EQI_VHEAVY;
					break;
				case EQI_GARMENT:
					sd->garmenttype = id->def < 2 ? EQI_LIGHT : id->def < 3 ? EQI_MEDIUM : id->def < 4 ? EQI_NEUTRAL : id->def < 6 ? EQI_HEAVY : EQI_VHEAVY;
					break;
				case EQI_SHOES:
					sd->shoestype = id->def < 2 ? EQI_LIGHT : id->def < 3 ? EQI_MEDIUM : id->def < 4 ? EQI_NEUTRAL : id->def < 6 ? EQI_HEAVY : EQI_VHEAVY;
					break;
			}
		}
	}
#endif
	script_pushint(st, 1);
	return SCRIPT_CMD_SUCCESS;
}
