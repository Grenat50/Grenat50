// Copyright (c) Mystic Project Dev Teams

#ifdef M5U00038
ACMD_FUNC(element)
{
	int param1 = 0;
	nullpo_retr(-1, sd);
	if (!message || !*message) {
		// notify the user of the requirement to enter an option
		clif_displaymessage(fd, "Please use @element <n> (1: Water, 2: Earth, 3: Fire, 4: Wind)");
		return -1;
	}
	sscanf(message, "%11d", &param1);
	if(param1 < 1 || param1 > 4) {
		clif_displaymessage(fd, "Please use @element <n> (1: Water, 2: Earth, 3: Fire, 4: Wind)");
		return -1;
	}
	pc_setreg(sd, add_str("@chosenele"), param1);
	return 0;
}
ACMD_FUNC(hat)
{
	int param1 = 0;
	nullpo_retr(-1, sd);
	clif_displaymessage(fd, "Please use @hat <n>(0: Choose, 1: Remove Upper, 2: Remove Middle, 3: Remove Lower, 4: Remove Garment, 5: All, 6: Remove Effect)");
	sscanf(message, "%11d", &param1);
	switch (param1) {
		default: case 0: clif_sendhat(sd,0); break;
		case 1: sd->hat_top = 0; pc_setglobalreg(sd, add_str("hat_top"), 0); clif_changelook(&sd->bl, LOOK_HEAD_TOP, sd->status.head_top); break;
		case 2: sd->hat_mid = 0; pc_setglobalreg(sd, add_str("hat_mid"), 0); clif_changelook(&sd->bl, LOOK_HEAD_MID, sd->status.head_mid); break;
		case 3: sd->hat_low = 0; pc_setglobalreg(sd, add_str("hat_low"), 0); clif_changelook(&sd->bl, LOOK_HEAD_BOTTOM, sd->status.head_bottom); break;
		case 4: sd->hat_robe = 0; pc_setglobalreg(sd, add_str("hat_robe"), 0); clif_changelook(&sd->bl, LOOK_ROBE, sd->status.robe); break;
		case 5:
			sd->hat_top = 0;
			sd->hat_mid = 0;
			sd->hat_low = 0;
			sd->hat_robe = 0;
			pc_setglobalreg(sd, add_str("hat_low"), 0);
			pc_setglobalreg(sd, add_str("hat_mid"), 0);
			pc_setglobalreg(sd, add_str("hat_top"), 0);
			pc_setglobalreg(sd, add_str("hat_robe"), 0);
			pc_setglobalreg(sd, add_str("hat_effect"), 0);
			clif_changelook(&sd->bl, LOOK_HEAD_BOTTOM, sd->status.head_bottom);
			clif_changelook(&sd->bl, LOOK_HEAD_MID, sd->status.head_mid);
			clif_changelook(&sd->bl, LOOK_HEAD_TOP, sd->status.head_top);
			clif_changelook(&sd->bl, LOOK_ROBE, sd->status.robe);
			break;
		case 6:
			pc_setglobalreg(sd, add_str("hat_effect"), 0);
			break;
	}
	return 0;
}
ACMD_FUNC(pet)
{
	int param1 = 0;
	nullpo_retr(-1, sd);
	sscanf(message, "%11d", &param1);
	clif_displaymessage(fd, "Please use @pet <n> (0: Choose Costume, 1: Remove Costume)");
	switch (param1) {
		case 0: default: clif_sendhat(sd,3); break;
		case 1:
		{
			struct pet_data *pd = sd->pd;
			if(pd) {
				sd->hat_pet = 0;
				pd->vd.class_ = pd->db->vd.class_;
				unit_refresh(&pd->bl);
				pc_setglobalreg(sd, add_str("hat_pet"), 0);
			}
			break;
		}
	}
	return 0;
}
#endif


#ifdef M5U00024
ACMD_FUNC(rs) {
	nullpo_retr(-1, sd);
		struct s_mapiterator* iter;
		struct map_session_data* pl_sd;
		sprintf(atcmd_output, "Server is reloading scripts... (Freeze for a few seconds)" );
		intif_broadcast(atcmd_output, strlen(atcmd_output) + 1, BC_DEFAULT);

		iter = mapit_getallusers();
		for( pl_sd = (TBL_PC*)mapit_first(iter); mapit_exists(iter); pl_sd = (TBL_PC*)mapit_next(iter) ){
			pc_close_npc(pl_sd,1);
			clif_cutin(pl_sd, "", 255);
			pl_sd->state.block_action &= ~(PCBLOCK_ALL ^ PCBLOCK_IMMUNE);
			bg_queue_leave(pl_sd);
		}
		mapit_free(iter);

		for (auto &bg : bg_queues) {
				for (auto &bg_sd : bg->teama_members)
					bg_team_leave(bg_sd, 0, false); // Kick Team A from battlegrounds
				for (auto &bg_sd : bg->teamb_members)
					bg_team_leave(bg_sd, 0, false); // Kick Team B from battlegrounds
				bg_queue_clear(bg, true);
		}

		flush_fifos();
		map_reloadnpc(true); // reload config files seeking for npcs
		script_reload();
		npc_reload2();

		clif_displaymessage(fd, msg_txt(sd,100)); // Scripts have been reloaded.
	return 0;
}
#endif

#ifdef M5U00004
ACMD_FUNC(summon2)
{
	char name[NAME_LENGTH];
	int mob_id = 0;
	enum special_flag flag = SF_EMBLEM;
	int amount = 1;
	struct mob_data *md;
	const char* event = "Summon2::OnStart";
	t_tick tick=gettick();

	nullpo_retr(-1, sd);

	if (!message || !*message || sscanf(message, "%23s %11d %11d", name, &amount, &flag) < 1)
	{
		clif_displaymessage(fd, msg_txt(sd,1225)); // Please enter a monster name (usage: @summon <monster name> {amount,elite flag}).
		return -1;
	}

	if ((mob_id = atoi(name)) == 0)
		mob_id = mobdb_searchname(name);
	if(mob_id == 0 || mobdb_checkid(mob_id) == 0)
	{
		clif_displaymessage(fd, msg_txt(sd,40));	// Invalid monster ID or name.
		return -1;
	}

	for(int i = 0; i < amount; i++) {
		md = mob_once_spawn_sub(&sd->bl, sd->bl.m, -1, -1, "--ja--", mob_id, event, SZ_SMALL, AI_ATTACK);

		if(!md)
			return -1;

		md->s_flag = (enum special_flag)(flag|SF_EMBLEM);
		md->stance = SUMM_FREE2;
		if(!(md->status.mode&MD_AGGRESSIVE))
			md->status.mode = static_cast<enum e_mode>(md->status.mode|MD_AGGRESSIVE);
		md->master_id=sd->bl.id;
		md->special_state.ai=AI_ATTACK;
		//md->deletetimer=add_timer(tick+(duration*60000),mob_timer_delete,md->bl.id,0);
		clif_specialeffect(&md->bl,EF_ENTRY2,AREA);
		mob_spawn(md);
		sc_start4(NULL,&md->bl, SC_MODECHANGE, 100, 1, 0, MD_AGGRESSIVE, 0, 60000);
		clif_displaymessage(fd, msg_txt(sd,39));	// All monster summoned!
	}

	return 0;
}
#endif


#ifdef M5J00006
/*==========================================
 * Makes the summon speak [Grenat]
 *------------------------------------------*/
ACMD_FUNC(sumtalk)
{
	char mes[100],temp[CHAT_SIZE_MAX];

	nullpo_retr(-1, sd);

	if ( battle_config.min_chat_delay ) {
		if( DIFF_TICK(sd->cantalk_tick, gettick()) > 0 )
			return 0;
		sd->cantalk_tick = gettick() + battle_config.min_chat_delay;
	}

	if (sd->sc.cant.chat || (sd->state.block_action & PCBLOCK_CHAT))
		return -1; //no "chatting" while muted.

	if ( !map_id2bl(sd->summon_id) ) {
		clif_displaymessage(fd, "Your summon is not active");
		return -1;
	}

	if (!message || !*message || sscanf(message, "%99[^\n]", mes) < 1) {
		clif_displaymessage(fd, "Please enter a message (usage: @sumtalk <message>)");
		return -1;
	}

	snprintf(temp, sizeof temp ,"%s : %s", mob_db.find(((TBL_MOB*)map_id2bl(sd->summon_id))->mob_id)->jname.c_str(), mes);
	clif_disp_overhead(map_id2bl(sd->summon_id), temp);

	return 0;
}

/*==========================================
 * Removes an invoked summon [Grenat]
 *------------------------------------------*/
ACMD_FUNC(sumremove)
{
	nullpo_retr(-1, sd);

	if ( !map_id2bl(sd->summon_id) ) {
		clif_displaymessage(fd, "Your summon is not active");
		return -1;
	}

	unit_free(map_id2bl(sd->summon_id),CLR_OUTSIGHT);
	sd->summon_id = 0;

	return 0;
}

/*==========================================
 * Makes the summon stop attacking and be passive [Grenat]
 *------------------------------------------*/
ACMD_FUNC(sumchill)
{
	nullpo_retr(-1, sd);

	struct mob_data *md = map_id2md(sd->summon_id);

	if ( !md ) {
		clif_displaymessage(fd, "Your summon is not active");
		return -1;
	}

	unit_stop_attack(&md->bl);
	md->target_id = 0;
	md->stance = SUMM_BASIC;
	md->status.mode = static_cast<enum e_mode>(md->status.mode&~(MD_CANATTACK|MD_AGGRESSIVE));
	status_calc_misc(&md->bl, &md->status, md->level);

	return 0;
}

/*==========================================
 * Makes the summon stop attacking and be passive [Grenat]
 *------------------------------------------*/
int mob_countslave_sub2(struct block_list *bl,va_list ap)
{
	int id;
	struct mob_data *md;
	id=va_arg(ap,int);

	md = (struct mob_data *)bl;
	if( md->master_id == id ) {
		struct map_session_data *sd = map_id2sd(id);
		char output[256];
		sprintf(output, "[%s Stats] : HP %d, ATK %d - %d, DEF %d, MDEF %d, MATK %d - %d, HIT %d, FLEE %d, CRI %d, PFLEE %d, Walking Speed %d, Attack per Second %d,%d(/s)", md->db->jname.c_str(), md->status.hp, md->status.rhw.atk, md->status.rhw.atk2, md->status.def, md->status.mdef, md->status.matk_min, md->status.matk_max, md->status.hit, md->status.flee, md->status.cri/10, md->status.flee2/10, (100 * DEFAULT_WALK_SPEED / (md->status.speed != 0 ? md->status.speed : 1)), (1000/(md->status.adelay != 0 ? md->status.adelay : 1)), (1000%(md->status.adelay != 0 ? md->status.adelay : 1)));
		if(sd)
			clif_displaymessage(sd->fd, output);
	}
	return 0;
}
ACMD_FUNC(suminfo)
{
	nullpo_retr(-1, sd);

	map_foreachinmap(mob_countslave_sub2, sd->bl.m, BL_MOB, sd->bl.id);

	return 0;
}
#endif


#ifdef M5U00054
ACMD_FUNC(linkitem)
{
	nullpo_retr(-1, sd);
	unsigned int nameid = atoi(message), refine = 0;
	if (!message || !*message || item_db.find(nameid) == NULL) {
		clif_displaymessage(fd, "Invalid Item ID.");
		return -1;
	}
	struct s_item_link itemldata;
	memset(&itemldata, 0, sizeof(itemldata));
	std::string itemlstr = createItemLink(nameid, refine, &itemldata);
	char *str = (char *)aMalloc((itemlstr.size() + 1) * sizeof(char));
	safestrncpy(str, itemlstr.c_str(), itemlstr.size() + 1);
	sprintf(atcmd_output, "%s : '%s'", sd->status.name, str);
	clif_disp_overhead(&sd->bl, atcmd_output);
	return 0;
}
#endif

#ifdef M5U00005
ACMD_FUNC(enchantui)
{
	nullpo_retr(-1, sd);

	if( sd->state.refineui_open ){
		clif_displaymessage(fd, msg_txt(sd, 775)); // You have already opened the refine UI.
		return -1;
	}

	clif_enchantui_open(sd);
	return 0;
}
#endif


#ifdef M5C00002
/*==========================================
* @afk
*------------------------------------------*/
ACMD_FUNC(afk) {
	nullpo_retr(-1, sd);
/*	if(sd->bl.m == map_mapname2mapid("prontera")) {
		clif_displaymessage(fd, "@afk is not allowed on this map.");
		return 0;
	}*/
				
	if( pc_isdead(sd) ) {
		clif_displaymessage(fd, "Cannot @afk if you are dead.");
		return -1;
	}

	if( map_getmapflag(sd->bl.m, MF_AUTOTRADE) == battle_config.autotrade_mapflag )
	{

		if ((map_getmapflag(sd->bl.m, MF_GVG)) || (map_getmapflag(sd->bl.m, MF_PVP))) {
			clif_displaymessage(fd, "You may not use the @afk maps PVP or GVG.");
			return -1;}

		sd->state.autotrade = 1;
		pc_setsit(sd);
		skill_sit(sd,1);
		clif_sitting(&sd->bl);
		clif_changelook(&sd->bl,LOOK_HEAD_TOP,471);
		clif_specialeffect(&sd->bl, 234,AREA);                       
		if( battle_config.afk_timeout )
		{
			int timeout = atoi(message);
			status_change_start(NULL, &sd->bl, SC_AUTOTRADE, 10000,0,0,0,0, ((timeout > 0) ? min(timeout,battle_config.afk_timeout) : battle_config.afk_timeout)*60000,0);
		}
			clif_authfail_fd(fd, 15);
		} else
			clif_displaymessage(fd, "@afk is not allowed on this map.");
	return 0;
}
#endif

#ifdef M5U00038

/*==========================================
* Remove hat from display
* @hat
*------------------------------------------*/
ACMD_FUNC(removehat) {
	nullpo_retr(-1, sd);
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
	clif_changelook(&sd->bl, LOOK_HEAD_TOP, sd->status.robe);
	return 0;
}
#endif

#ifdef M5U00055
ACMD_FUNC(init)
{
	nullpo_retr(-1,sd);
	t_tick tick = gettick();

	if(!&sd->sc || !sd->sc.data[SC_INIT])
		clif_displaymessage(fd, "Init state: Yes");
	else
		clif_displaymessage(fd, "Init state: No");

	return 0;
}
#endif
#ifdef M5U00034
/*==========================================
* Battleground Leader Commands
*------------------------------------------*/
ACMD_FUNC(order)
{
	nullpo_retr(-1,sd);
	memset(atcmd_output, '\0', sizeof(atcmd_output));
	if( !message || !*message )
	{
		clif_displaymessage(fd, "Please, enter a message (usage: @order <message>).");
		return -1;
	}

	if( map_getmapflag(sd->bl.m, MF_BATTLEGROUND) )
	{
		std::shared_ptr<s_battleground_data> bgteam = util::umap_find(bg_team_db, sd->bg_id);
		if( bgteam->leader_char_id !=  sd->status.char_id) {
			clif_displaymessage(fd, "This command is reserved for Team Leaders Only.");
			return -1;
		}
		if (battle_config.bg_order_behavior)
			sprintf(atcmd_output, "%s: %s", sd->status.name, message);
		else
			sprintf(atcmd_output, "Team Leader: %s", message);
		clif_broadcast2(&sd->bl, atcmd_output, (int)strlen(atcmd_output)+1, bgteam->color, 0x190, 20, 0, 0, BG);
	}
	else
	{
		if( !sd->state.gmaster_flag )
		{
			clif_displaymessage(fd, "This command is reserved for Guild Leaders Only.");
			return -1;
		}
		clif_broadcast2(&sd->bl, message, (int)strlen(message)+1, 0xFF0000, 0x190, 20, 0, 0, GUILD);
	}

	return 0;
}

ACMD_FUNC(leader)
{
	struct map_session_data *pl_sd = NULL;
	nullpo_retr(-1,sd);
	memset(atcmd_player_name, '\0', sizeof(atcmd_player_name));

	if (!message || !*message || sscanf(message, "%23[^\n]", atcmd_player_name) < 1) {
		clif_displaymessage(fd,"Please enter a player name (usage: @leader <char name/ID>).");
		return -1;
	}
	if(!sd->bg_id) {
		clif_displaymessage(fd, "This command is reserved for Battleground Only.");
		return -1;
	}
	std::shared_ptr<s_battleground_data> bgd = util::umap_find(bg_team_db, sd->bg_id);
	if( bgd->leader_char_id !=  sd->status.char_id)
		clif_displaymessage(fd, "This command is reserved for Team Leaders Only.");
	else if( !bgd || !sd->bg_id)
		clif_displaymessage(fd, "This command is reserved for Battleground Only.");
	else if( sd->ud.skilltimer != INVALID_TIMER )
		clif_displaymessage(fd, "Command not allow while casting a skill.");
	else if( !message || !*message )
		clif_displaymessage(fd, "Please, enter the new Leader name (usage: @leader <name>).");
	else if((pl_sd=map_nick2sd(atcmd_player_name,true)) == NULL && (pl_sd=map_charid2sd(atoi(atcmd_player_name))) == NULL)
		clif_displaymessage(fd, "Character not found.");
	else if( sd->bg_id != pl_sd->bg_id )
		clif_displaymessage(fd, "Destination Player is not in your Team.");
	else if( sd == pl_sd )
		clif_displaymessage(fd, "You are already the Team Leader.");
	else
	{ // Everytest OK!
		sprintf(atcmd_output, "Team Leader transfered to [%s]", pl_sd->status.name);
		clif_broadcast2(&sd->bl, atcmd_output, (int)strlen(atcmd_output)+1, bgd->color, 0x190, 20, 0, 0, BG_LISTEN);

		bgd->leader_char_id = pl_sd->status.char_id;

		clif_name_area(&sd->bl);
		clif_name_area(&pl_sd->bl);
		return 0;
	}
	return -1;
}

ACMD_FUNC(reportafk)
{
	struct map_session_data *pl_sd = NULL;
	nullpo_retr(-1,sd);
	memset(atcmd_player_name, '\0', sizeof(atcmd_player_name));

	if (!message || !*message || sscanf(message, "%23[^\n]", atcmd_player_name) < 1) {
		clif_displaymessage(fd,"Please enter a player name (usage: @reportafk <char name/ID>).");
		return -1;
	}
	std::shared_ptr<s_battleground_data> bgteam = util::umap_find(bg_team_db, sd->bg_id);
	if( !sd->bg_id )
		clif_displaymessage(fd, "This command is reserved for Battleground Only.");
	else if( bgteam->leader_char_id != sd->status.char_id && battle_config.bg_reportafk_leaderonly )
		clif_displaymessage(fd, "This command is reserved for Team Leaders Only.");
	else if( !message || !*message )
		clif_displaymessage(fd, "Please, enter the character name (usage: @reportafk <name>).");
	else if((pl_sd=map_nick2sd(atcmd_player_name,true)) == NULL && (pl_sd=map_charid2sd(atoi(atcmd_player_name))) == NULL)
		clif_displaymessage(fd, "Character not found");
	else if( sd->bg_id != pl_sd->bg_id )
		clif_displaymessage(fd, "Destination Player is not in your Team.");
	else if( sd == pl_sd )
		clif_displaymessage(fd, "You cannot kick yourself.");
	else if( pl_sd->state.bg_afk == 0 )
		clif_displaymessage(fd, "The player is not AFK on this Battleground.");
	else
	{ // Everytest OK!
		if( bgteam == NULL )
			return -1;

		bg_team_leave(pl_sd,2,true);
		clif_displaymessage(pl_sd->fd, "You have been kicked from Battleground because of your AFK status.");
		pc_setpos(pl_sd,pl_sd->status.save_point.map,pl_sd->status.save_point.x,pl_sd->status.save_point.y,CLR_TELEPORT);

		sprintf(atcmd_output, "- AFK [%s] Kicked by @reportafk command-", pl_sd->status.name);
		clif_broadcast2(&sd->bl, atcmd_output, (int)strlen(atcmd_output)+1, bgteam->color, 0x190, 20, 0, 0, BG);
		return 0;
	}
	return -1;
}

ACMD_FUNC(listenbg)
{
	if (sd->state.bg_listen)
	{
		sd->state.bg_listen = 0;
		clif_displaymessage(fd, "You will receive Battleground announcements");
	}
	else
	{
		sd->state.bg_listen = 1;
		clif_displaymessage(fd, "You will not receive Battleground announcements.");
	}

	return 0;
}

/*==========================================
* Guild Skill Usage for Guild Masters
*------------------------------------------*/
ACMD_FUNC(bgskill)
{
	if(!sd->bg_id) return -1;
	int i, skillnum = 0, skilllv = 0;
	t_tick tick = gettick();
	std::shared_ptr<s_battleground_data> bgd = util::umap_find(bg_team_db, sd->bg_id);
	const struct { char skillstr[3]; int id; } skills[] = {
		{ "BO",	10010 },
		{ "RG",	10011 },
		{ "RS",	10012 },
		{ "EC",	10013 },
		{ "CF",	10017 },
		{ "CB",	10018 },
		{ "EM",	10019 },
	};

	// Check for Skill ID
	for( i = 0; i < ARRAYLENGTH(skills); i++ )
	{
		if( strncmpi(message, skills[i].skillstr, 3) == 0 )
		{
			skillnum = skills[i].id;
			break;
		}
	}
	if( !skillnum )
	{
		clif_displaymessage(fd, "Invalid Skill string. Use @bgskill EC/RS/RG/BO/CF/CB/EM");
		return -1;
	}

	if( !map_getmapflag(sd->bl.m, MF_BATTLEGROUND) )
	{
		clif_displaymessage(fd, "This command is only available for Battleground.");
		return -1;
	}
	else
	{
		if( bgd->leader_char_id == sd->status.char_id )
		{
			if(bg_block_skill_status(sd, skillnum))
				return -1;
			else
				skilllv = 1;
		}
		else
		{
			clif_displaymessage(fd, "This command is reserved for Team Leaders Only.");
			return -1;
		}
	}

	if( pc_cant_act(sd) || pc_issit(sd) || sd->sc.option&(OPTION_WEDDING|OPTION_XMAS|OPTION_SUMMER) || sd->state.only_walk || sd->sc.data[SC_BASILICA] )
		return -1;

	if( DIFF_TICK(tick, sd->ud.canact_tick) < 0 )
		return -1;

	if( sd->menuskill_id )
	{
		if( sd->menuskill_id == SA_TAMINGMONSTER )
			sd->menuskill_id = sd->menuskill_val = 0; //Cancel pet capture.
		else if( sd->menuskill_id != SA_AUTOSPELL )
			return -1; //Can't use skills while a menu is open.
	}

	sd->skillitem = sd->skillitemlv = 0;
	if(skillnum)
		unit_skilluse_id(&sd->bl, sd->bl.id, skillnum, 1);
	return 0;
}
#endif

#ifdef M5U00064
/* Battlestats
   1: Offensive, 2: Defensive, 3: Utility, 4: Regeneration, 5: All
*/
void atcommand_battlestats_sub(int fd, struct map_session_data *sd, int type) {
	status_data *status = status_get_status_data(&sd->bl);
	char output[CHAT_SIZE_MAX];
	int numberMin = 0, numberMax = 0, j = 0;

#define battlestats_msg(__msg__) (__msg__); clif_displaymessage(fd, output)

	battlestats_msg(sprintf(output, "Battle Status: %s", sd->status.name));
	struct guild* g = guild_search(sd->status.guild_id);

	// Offensive Stats
	clif_displaymessage(fd, "|--- Basic Info ---|");
	strcpy(output, "");
	if(type == 1 || type == 0) {
		// battlestats_msg(sprintf(output, "   -> ATK: base %d + %d + %d + %d + %d (+ %d%%)", sd->battle_status.batk, sd->battle_status.rhw.atk, sd->battle_status.lhw.atk, sd->battle_status.rhw.atk2, sd->battle_status.lhw.atk2, sd->bonus.atk_rate));
		battlestats_msg(sprintf(output, "   -> ATK: %d ~ %d", calc_pc_min_atk(&sd->battle_status), calc_pc_max_atk(&sd->battle_status)));
		battlestats_msg(sprintf(output, "   -> MATK: %d ~ %d", pc_leftside_matk(sd), pc_rightside_matk(sd)));
		battlestats_msg(sprintf(output, "   -> HIT: %d, perfect HIT: %d%%", status->hit, sd->bonus.perfect_hit));
		battlestats_msg(sprintf(output, "   -> FLEE: %d, perfect FLEE: %d%%", status->flee, status->flee2 / 10));
		battlestats_msg(sprintf(output, "   -> CRIT: %d, CRIT DEF: %d%%, CRIT DMG: +%d%%", status->cri / 10,sd->bonus.critical_def,sd->bonus.crit_atk_rate));
		if(status->amotion > 0)
			battlestats_msg(sprintf(output, "   -> ASPD: %d.%d (%d,%d/sec)", ((2000 - status->amotion) / 10),((2000 - status->amotion) - (((2000 - status->amotion) /10)*10)), (500/(2000 - status->amotion)), (500%(2000 - status->amotion))));
	}
	if(type == 2 || type == 0) {
		battlestats_msg(sprintf(output, "   -> DEF (%%) : %d%%, DEF2 (fix): %d", pc_leftside_def(sd), pc_rightside_def(sd)));
		battlestats_msg(sprintf(output, "   -> MDEF (%%) : %d%%, MDEF2 (fix): %d", pc_leftside_mdef(sd), pc_rightside_mdef(sd)));
	}
	if (type == 3 || type == 0) {
		if(sd->battle_status.speed > 0)
			battlestats_msg(sprintf(output, "   -> Walking Speed: %d%%", 100 * DEFAULT_WALK_SPEED / sd->battle_status.speed));
	}
	if(type == 4 || type == 0) {
		battlestats_msg(sprintf(output, "   -> Regen HP Rate: %d%%", sd->hprecov_rate));
		battlestats_msg(sprintf(output, "   -> Regen SP Rate: %d%%", sd->sprecov_rate));
		battlestats_msg(sprintf(output, "   -> Regen LP Rate: %d%%", 100+(int)sd->special_state.limit_point_recovery));
	}
	clif_displaymessage(fd, "|--- Bonus from Stuff ---|");
	strcpy(output, "");
	struct {
		const char *format;
		int value;
	} bonus_info[] = {
		{ "   -> ATK: +%d%%", sd->bonus.atk_rate },
		{ "   -> MATK: +%d%%", sd->matk_rate-100 },
		{ "   -> MATKmin: +%d", (int)sd->special_state.matk_min },
		{ "   -> MATKmax: +%d", (int)sd->special_state.matk_max },
		{ "   -> Double Atk rate (skill excluded): %d%%", sd->bonus.double_rate },
		{ "   -> ASPD bonus: +%d", 10*sd->bonus.aspd_add },
		{ "   -> ASPD bonus: +%d%%", 100-(sd->battle_status.aspd_rate/10) },
		{ "   -> ASPD max: +%d(x0.1)", (int)sd->special_state.maximum_aspd_rate },
		{ "   -> Critical Damage: +%d%%", sd->bonus.crit_atk_rate },
		{ "   -> Critical Chance: +%d%%", sd->critical_rate-100 },
		{ "   -> Long Distance Damage: +%d%%", sd->bonus.long_attack_atk_rate },
		{ "   -> Break Weapon: %d%%", sd->bonus.break_weapon_rate/100 },
		{ "   -> Break Armor: %d%%", sd->bonus.break_armor_rate/100 },
		{ "   -> Break Weapon (npc): %d%%", sd->special_state.break_weapon_rate_npc/100 },
		{ "   -> Break Armor (npc): %d%%", sd->special_state.break_armor_rate_npc/100 },
		{ "   -> Damage from Back: +%d%%", sd->special_state.physical_back_rate },
		{ "   -> Damage from Front: +%d%%", sd->special_state.physical_front_rate },
		{ "   -> Damage if Target HP > 50%: +%d%%", (int)sd->special_state.first_add_rate },
		{ "   -> Damage if Target HP < 50%: +%d%%", (int)sd->special_state.last_add_rate },
		{ "   -> Damage DOT: +%d%%", (int)sd->special_state.dot_add_rate },
		{ "   -> Damage if Target can die: +%d%%", (int)sd->special_state.final_add_rate },
		{ "   -> Damage to any kind of damage: +%d%%", (int)sd->special_state.all_damage_rate },
		{ "   -> Critical Skills: +%d%%", (int)sd->special_state.high_crit_rate },
		{ "   -> 1 vs 1 Damage: +%d%%", (int)sd->special_state.physical_dual_add_rate },
		{ "   -> Multiple Enemies Damage: +%d%%", (int)sd->special_state.physical_multiple_add_rate },
		{ "   -> Ranged damage at close range (phy): +%d%%", (int)sd->special_state.add_short_range_damage[1] },
		{ "   -> Ranged resistance at close range (phy): +%d%%", (int)sd->special_state.sub_short_range_damage[1] },
		{ "   -> Ranged damage at long range (phy): +%d%%", (int)sd->special_state.add_long_range_damage[1] },
		{ "   -> Ranged resistance at long range (phy): +%d%%", (int)sd->special_state.sub_long_range_damage[1] },
		{ "   -> Ranged damage at close range (mag): +%d%%", (int)sd->special_state.add_short_range_damage[2] },
		{ "   -> Ranged resistance at close range (mag): +%d%%", (int)sd->special_state.sub_short_range_damage[2] },
		{ "   -> Ranged damage at long range (mag): +%d%%", (int)sd->special_state.add_long_range_damage[2] },
		{ "   -> Ranged resistance at long range (mag): +%d%%", (int)sd->special_state.sub_long_range_damage[2] },
		{ "   -> Ranged damage at close range (spe): +%d%%", (int)sd->special_state.add_short_range_damage[3] },
		{ "   -> Ranged resistance at close range (spe): +%d%%", (int)sd->special_state.sub_short_range_damage[3] },
		{ "   -> Ranged damage at long range (spe): +%d%%", (int)sd->special_state.add_long_range_damage[3] },
		{ "   -> Ranged resistance at long range (spe): +%d%%", (int)sd->special_state.sub_long_range_damage[3] },
		{ "   -> Summon Power Rate: +%d%%", (int)sd->special_state.summon_power_rate },
		{ "   -> Damage on Summons: +%d%%", (int)sd->special_state.summon_add_rate },
		{ "   -> Critical Defense: +%d%%", sd->bonus.critical_def },
		{ "   -> Reflect Short: %d%%", sd->bonus.short_weapon_damage_return },
		{ "   -> Reflect Long: %d%%", sd->bonus.long_weapon_damage_return },
		{ "   -> Reflect Magic: %d%%", sd->bonus.magic_damage_return+(int)sd->special_state.magical_damage_return },
		{ "   -> Reflect Misc: %d%%", (int)sd->special_state.special_damage_return },
		{ "   -> Damage Absorb: %d%%", sd->bonus.absorb_dmg_maxhp },
		{ "   -> Devotion HP lost: -%d%%", (int)sd->special_state.devotion_sub_rate },
		{ "   -> HP lost Sacrifice: -%d%%", (int)sd->special_state.sacrifice_sub_rate },
		{ "   -> Common Status Chance (on self): -%d%%", (int)sd->special_state.status_sub_success },
		{ "   -> Common Status Duration (on self): -%d%%", (int)sd->special_state.status_sub_duration },
		{ "   -> Damage Reduction if HP > 50%: up to %d%%", (int)sd->special_state.first_sub_rate },
		{ "   -> Damage Reduction if HP < 50%: up to %d%%", (int)sd->special_state.last_sub_rate },
		{ "   -> Damage DOT: -%d%%", (int)sd->special_state.dot_sub_rate },
		{ "   -> Damage if you can die: -%d%%", (int)sd->special_state.final_sub_rate },
		{ "   -> FLEE if HP low: up to +%d%%", (int)sd->special_state.low_hp_avoid_rate },
		{ "   -> HIT if HP low: up to +%d%%", (int)sd->special_state.low_hp_sight_rate },
		{ "   -> Crit if HP low: up to +%d%%", (int)sd->special_state.low_hp_crit_rate },
		{ "   -> ATK if HP low: up to +%d%%", (int)sd->special_state.low_hp_atk_rate },
		{ "   -> MATK if SP low: up to +%d%%", (int)sd->special_state.low_sp_matk_rate },
		{ "   -> No Pushback: %d%%", (int)sd->special_state.no_knockback_rate },
		{ "   -> After Hit Recovery: +%d%%", (int)sd->special_state.after_hit_recovery },
		{ "   -> Magical Flee (1 dmg): %d%%", (int)sd->special_state.magical_flee_rate },
		{ "   -> Blocking Rate (phy): %d%%", (int)sd->special_state.blocking_rate_all+(int)sd->special_state.blocking_rate_phy },
		{ "   -> Blocking Rate (mag): %d%%", (int)sd->special_state.blocking_rate_all+(int)sd->special_state.blocking_rate_mag },
		{ "   -> Blocking Rate (spe): %d%%", (int)sd->special_state.blocking_rate_all+(int)sd->special_state.blocking_rate_spe },
		{ "   -> Blocking Capacity: %d%%", (int)sd->special_state.blocking_capacity },
		{ "   -> Parrying Rate (phy): %d%%", (int)sd->special_state.parrying_rate_all+(int)sd->special_state.parrying_rate_phy },
		{ "   -> Parrying Rate (mag): %d%%", (int)sd->special_state.parrying_rate_all+(int)sd->special_state.parrying_rate_mag },
		{ "   -> Parrying Rate (spe): %d%%", (int)sd->special_state.parrying_rate_all+(int)sd->special_state.parrying_rate_spe },
		{ "   -> Parrying Capacity: %d%%", (int)sd->special_state.parrying_capacity },
		{ "   -> Front Defense: %d%%", (int)sd->special_state.front_def_rate },
		{ "   -> Damage Reduction while Casting: %d%%", (int)sd->special_state.cast_def_rate },
		{ "   -> 1 on 1 Damage: -%d%%", (int)sd->special_state.physical_dual_sub_rate },
		{ "   -> Damage Reduction with Multiple Enemies: %d%%", (int)sd->special_state.physical_multiple_sub_rate },
		{ "   -> Damage into SP: %d%%", (int)sd->special_state.damage_to_sp },
		{ "   -> Skills SP cost into HP: %d%%", (int)sd->special_state.cost_to_hp_ratio },
		{ "   -> Avoid Physical (0 dmg): %d%%", (int)sd->special_state.flee_phy },
		{ "   -> Avoid Magical (0 dmg): %d%%", (int)sd->special_state.flee_mag },
		{ "   -> Avoid Misc (0 dmg): %d%%", (int)sd->special_state.flee_spe },
		{ "   -> First Hit Avoid Chance: %d%%", (int)sd->special_state.first_hit_avoid_rate },
		{ "   -> Damage from Summons: -%d%%", (int)sd->special_state.summon_sub_rate },
		{ "   -> Initiative CD: -%d%%", (int)sd->special_state.initiative_rate },
		{ "   -> SP Cost: -%d%%", sd->dsprate-100 },
		{ "   -> Walking Speed: +%d%%", -(sd->bonus.speed_rate + sd->bonus.speed_add_rate) },
		{ "   -> HP: +%d%%", sd->hprate-100 },
		{ "   -> SP: +%d%%", sd->sprate-100 },
		{ "   -> LP: +%d%%", (int)sd->special_state.limit_point_rate },
		{ "   -> HP: +%d", sd->bonus.hp },
		{ "   -> SP: +%d", sd->bonus.sp },
		{ "   -> Item Heal: +%d", sd->bonus.itemhealrate2 },
		{ "   -> Heal Received: +%d", sd->bonus.add_heal2_rate },
		{ "   -> Heal Inflicted: +%d", sd->bonus.add_heal_rate },
		{ "   -> CAST Reduction: %d%%", 100-sd->castrate },
		{ "   -> Delay Reduction: %d%%", sd->bonus.delayrate },
		{ "   -> Zeny Gain: +%d%%", sd->bonus.get_zeny_rate + (g ? (guild_checkskill(g,GD_GUARDRESEARCH) * 3) : 0) },
		{ "   -> Zeny Gain: +%d", sd->bonus.get_zeny_num },
		{ "   -> Zeny Gain: +%d%%", (int)sd->special_state.add_zeny_rate },
		{ "   -> SpellBreaker inflicted CD: +%d%%", (int)sd->special_state.spellbreaker_cd_rate },
		{ "   -> Status Rate (on target): +%d%%", (int)sd->special_state.status_add_success },
		{ "   -> Status Duration (on target): +%d%%", (int)sd->special_state.status_add_duration },
		{ "   -> No Cancel Cast Rate: +%d%%", (int)sd->special_state.no_cancel_rate },
		{ "   -> No Gem Requirement: %d%%", (int)sd->special_state.no_gem_requirement },
		{ "   -> No Ammo Requirement: %d%%", (int)sd->special_state.no_ammo_requirement },
		{ "   -> No Spirit Requirement: %d%%", (int)sd->special_state.no_spirit_requirement },
		{ "   -> No Zeny Requirement: %d%%", (int)sd->special_state.no_zeny_requirement },
		{ "   -> No Bolt Scroll Use: %d%%", (int)sd->special_state.no_bolt_scroll_use_rate },
		{ "   -> No Spell Scroll Use: %d%%", (int)sd->special_state.no_spell_scroll_use_rate },
		{ "   -> Dancing Speed Rate: +%d%%", (int)sd->special_state.perform_speed_rate },
		{ "   -> Drain HP attacking : %d% (x0.1%)", (int)sd->special_state.drain_hp_rate_all },
		{ "   -> On Hit HP Recovery (1s): %d% (x0.1%)", (int)sd->special_state.on_hit_hp_recovery_all },
		{ "   -> On Hit SP Recovery (1s): %d% (x0.1%)", (int)sd->special_state.on_hit_sp_recovery },
		{ "   -> Ignore Stuff Immunity: %d%%", (int)sd->special_state.ignore_stuff_immunity },
		{ "   -> Equipment Resistance (to break/strip): %d%%", (int)sd->special_state.stuff_resist_rate_all },
		{ "   -> Attributes bonus from Skills: +%d%%", (int)sd->special_state.attribute_bonus_rate },
		{ "   -> Dancing Bonuses: +%d%%", (int)sd->special_state.perform_bonus_rate },
		{ "   -> Dancing Duration: +%d%%", (int)sd->special_state.perform_duration_rate },
		{ "   -> Dancing SP cost: -%d%%", (int)sd->special_state.perform_sp_use_rate },
		{ "   -> Effects duration Skills: +%d%%", (int)sd->special_state.effect_duration_rate },
		{ "   -> Speed Penalties: -%d%%", (int)sd->special_state.minimum_speed_rate },
		{ "   -> Maximum Weight: +%d%%", (int)sd->special_state.maximum_weight_rate },
		{ "   -> Maximum Weight: +%d", (int)sd->special_state.maximum_weight_bonus },
		{ "   -> Weight: +%d%%", (int)sd->special_state.add_weight },
		{ "   -> Convert SP to Damage (Max SP): %d0%%", (int)sd->special_state.sp_to_damage },
		{ "   -> Convert SP to Damage (Bonus Rate): %d%%", (int)sd->special_state.sp_to_damage_rate },
		{ "   -> Convert SP to Damage (Ratio sp:dmg): %d%%", (int)sd->special_state.sp_to_damage_ratio },
		{ "   -> Convert HP to Damage (Max SP): %d/10%%", (int)sd->special_state.hp_to_damage },
		{ "   -> Convert HP to Damage (Bonus Rate): %d/10%%", (int)sd->special_state.hp_to_damage_rate },
		{ "   -> Convert HP to Damage (Ratio sp:dmg): %d/10%%", (int)sd->special_state.hp_to_damage_ratio },
		{ "   -> Sneak Attack Damage: +%d%%", (int)sd->special_state.sneak_rate },
		{ "   -> Map Item Rate: +%d%%", (int)sd->special_state.add_mapitem_rate + (g ? (guild_checkskill(g,GD_DEVELOPMENT) * 2) : 0) },
		{ "   -> Map Item Quality: +%d%%", (int)sd->special_state.add_mapitem_quality + g ? getgdbranchlv(g,GD_DEVELOPMENT) : 0 },
		{ "   -> Flee Magical: +%d%%", (int)sd->special_state.flee_mag },
		{ "   -> Flee Physical: +%d%%", (int)sd->special_state.flee_phy },
		{ "   -> Flee Special: +%d%%", (int)sd->special_state.flee_spe },
	};
	switch (type) {
		case 0: numberMin = 0; numberMax = ARRAYLENGTH(bonus_info); break;
		case 1: numberMin = 0; numberMax = 24; break;
		case 2: numberMin = 24; numberMax = 60; break;
		case 3: numberMin = 60; numberMax = ARRAYLENGTH(bonus_info); break;
		case 4: numberMin = 60; numberMax = ARRAYLENGTH(bonus_info); break;
	}
	for (int i = numberMin; i < numberMax; i++) {
		if (bonus_info[i].value == 0)
			continue;
		battlestats_msg(sprintf(output, bonus_info[i].format, bonus_info[i].value));
	}

	// Utility Stats
	if(type == 3 || type == 0) {
		clif_displaymessage(fd, "|--- Race Related ---|");
		strcpy(output, "");
		struct {
			const char *format;
			int value;
		} race_related_info[] = {
			{ "   -> EXP Formless: %d%%", sd->indexed_bonus.expaddrace[RC_FORMLESS] },
			{ "   -> EXP Undead: %d%%", sd->indexed_bonus.expaddrace[RC_UNDEAD] },
			{ "   -> EXP Beast: %d%%", sd->indexed_bonus.expaddrace[RC_BRUTE] },
			{ "   -> EXP Plant: %d%%", sd->indexed_bonus.expaddrace[RC_PLANT] },
			{ "   -> EXP Insect: %d%%", sd->indexed_bonus.expaddrace[RC_INSECT] },
			{ "   -> EXP Fish: %d%%", sd->indexed_bonus.expaddrace[RC_FISH] },
			{ "   -> EXP Demon: %d%%", sd->indexed_bonus.expaddrace[RC_DEMON] },
			{ "   -> EXP DemiHuman: %d%%", sd->indexed_bonus.expaddrace[RC_DEMIHUMAN] },
			{ "   -> EXP Angel: %d%%", sd->indexed_bonus.expaddrace[RC_ANGEL] },
			{ "   -> EXP Dragon: %d%%", sd->indexed_bonus.expaddrace[RC_DRAGON] },
			{ "   -> EXP Player: %d%%", sd->indexed_bonus.expaddrace[RC_PLAYER] },
			{ "   -> EXP ALL: %d%%", sd->indexed_bonus.expaddrace[RC_ALL] + (g ? (guild_checkskill(g,GD_KAFRACONTRACT) * 2) : 0) },
			{ "   -> Drop Formless: %d%%", sd->indexed_bonus.dropaddrace[RC_FORMLESS] },
			{ "   -> Drop Undead: %d%%", sd->indexed_bonus.dropaddrace[RC_UNDEAD] },
			{ "   -> Drop Beast: %d%%", sd->indexed_bonus.dropaddrace[RC_BRUTE] },
			{ "   -> Drop Plant: %d%%", sd->indexed_bonus.dropaddrace[RC_PLANT] },
			{ "   -> Drop Insect: %d%%", sd->indexed_bonus.dropaddrace[RC_INSECT] },
			{ "   -> Drop Fish: %d%%", sd->indexed_bonus.dropaddrace[RC_FISH] },
			{ "   -> Drop Demon: %d%%", sd->indexed_bonus.dropaddrace[RC_DEMON] },
			{ "   -> Drop DemiHuman: %d%%", sd->indexed_bonus.dropaddrace[RC_DEMIHUMAN] },
			{ "   -> Drop Angel: %d%%", sd->indexed_bonus.dropaddrace[RC_ANGEL] },
			{ "   -> Drop Dragon: %d%%", sd->indexed_bonus.dropaddrace[RC_DRAGON] },
			{ "   -> Drop Player: %d%%", sd->indexed_bonus.dropaddrace[RC_PLAYER] },
			{ "   -> Drop ALL: %d%%", sd->indexed_bonus.dropaddrace[RC_ALL] },
		};
		for (int i = 0; i < ARRAYLENGTH(race_related_info); i++) {
			if (race_related_info[i].value == 0)
				continue;
			battlestats_msg(sprintf(output, race_related_info[i].format, race_related_info[i].value));
		}

		clif_displaymessage(fd, "|--- Class and Size Related ---|");
		strcpy(output, "");
		struct {
			const char *format;
			int value;
		} class_related_info[] = {
			{ "   -> EXP Normal: %d%%", sd->indexed_bonus.expaddclass[CLASS_NORMAL] },
			{ "   -> EXP Boss: %d%%", sd->indexed_bonus.expaddclass[CLASS_BOSS] },
			{ "   -> EXP All: %d%%", sd->indexed_bonus.expaddclass[CLASS_ALL]  },
			{ "   -> EXP Small: %d%%", sd->special_state.exp_add_size[SZ_SMALL] },
			{ "   -> EXP Medium: %d%%", sd->special_state.exp_add_size[SZ_MEDIUM] },
			{ "   -> EXP Big: %d%%", sd->special_state.exp_add_size[SZ_BIG] },
			{ "   -> EXP Size All: %d%%", sd->special_state.exp_add_size[SZ_ALL]  },
			{ "   -> Drop Normal: %d%%", sd->indexed_bonus.dropaddclass[CLASS_NORMAL] },
			{ "   -> Drop Boss: %d%%", sd->indexed_bonus.dropaddclass[CLASS_BOSS] },
			{ "   -> Drop All: %d%%", sd->indexed_bonus.dropaddclass[CLASS_ALL] + (g ? getgdbranchlv(g,GD_GUARDRESEARCH) : 0) },
		};
		for (int i = 0; i < ARRAYLENGTH(class_related_info); i++) {
			if (class_related_info[i].value == 0)
				continue;
			battlestats_msg(sprintf(output, class_related_info[i].format, class_related_info[i].value));
		}
	}

	// Regeneration Stats
	if(type == 4 || type == 0) {
		clif_displaymessage(fd, "|--- Bonus Regen ---|");
		strcpy(output, "");
		if (sd->hp_regen.value)
			battlestats_msg(sprintf(output, "   -> Regen HP: %d%%", sd->hp_regen.value));
		if (sd->sp_regen.value)
			battlestats_msg(sprintf(output, "   -> Regen SP: %d%%", sd->sp_regen.value));
	}

	if (type == 1 || type == 0) {
		clif_displaymessage(fd, "|--- Elemental Bonus Damage ---|");
		strcpy(output, "");
		struct {
			const char* format;
			int value;
		} ele_bonus_damage_info[] = {
			{ "   -> Physical Neutral: %d%%", (int)sd->special_state.element_damage_rate_phy[ELE_NEUTRAL] },
			{ "   -> Physical Water: %d%%", (int)sd->special_state.element_damage_rate_phy[ELE_WATER] },
			{ "   -> Physical Earth: %d%%", (int)sd->special_state.element_damage_rate_phy[ELE_EARTH] },
			{ "   -> Physical Fire: %d%%", (int)sd->special_state.element_damage_rate_phy[ELE_FIRE] },
			{ "   -> Physical Wind: %d%%", (int)sd->special_state.element_damage_rate_phy[ELE_WIND] },
			{ "   -> Physical Poison: %d%%", (int)sd->special_state.element_damage_rate_phy[ELE_POISON] },
			{ "   -> Physical Holy: %d%%", (int)sd->special_state.element_damage_rate_phy[ELE_HOLY] },
			{ "   -> Physical Dark: %d%%", (int)sd->special_state.element_damage_rate_phy[ELE_DARK] },
			{ "   -> Physical Ghost: %d%%", (int)sd->special_state.element_damage_rate_phy[ELE_GHOST] },
			{ "   -> Physical Undead: %d%%", (int)sd->special_state.element_damage_rate_phy[ELE_UNDEAD] },
			{ "   -> Magical Neutral: %d%%", (int)sd->special_state.element_damage_rate_mag[ELE_NEUTRAL] },
			{ "   -> Magical Water: %d%%", (int)sd->special_state.element_damage_rate_mag[ELE_WATER] },
			{ "   -> Magical Earth: %d%%", (int)sd->special_state.element_damage_rate_mag[ELE_EARTH] },
			{ "   -> Magical Fire: %d%%", (int)sd->special_state.element_damage_rate_mag[ELE_FIRE] },
			{ "   -> Magical Wind: %d%%", (int)sd->special_state.element_damage_rate_mag[ELE_WIND] },
			{ "   -> Magical Poison: %d%%", (int)sd->special_state.element_damage_rate_mag[ELE_POISON] },
			{ "   -> Magical Holy: %d%%", (int)sd->special_state.element_damage_rate_mag[ELE_HOLY] },
			{ "   -> Magical Dark: %d%%", (int)sd->special_state.element_damage_rate_mag[ELE_DARK] },
			{ "   -> Magical Ghost: %d%%", (int)sd->special_state.element_damage_rate_mag[ELE_GHOST] },
			{ "   -> Magical Undead: %d%%", (int)sd->special_state.element_damage_rate_mag[ELE_UNDEAD] },
			{ "   -> Special Neutral: %d%%", (int)sd->special_state.element_damage_rate_spe[ELE_NEUTRAL] },
			{ "   -> Special Water: %d%%", (int)sd->special_state.element_damage_rate_spe[ELE_WATER] },
			{ "   -> Special Earth: %d%%", (int)sd->special_state.element_damage_rate_spe[ELE_EARTH] },
			{ "   -> Special Fire: %d%%", (int)sd->special_state.element_damage_rate_spe[ELE_FIRE] },
			{ "   -> Special Wind: %d%%", (int)sd->special_state.element_damage_rate_spe[ELE_WIND] },
			{ "   -> Special Poison: %d%%", (int)sd->special_state.element_damage_rate_spe[ELE_POISON] },
			{ "   -> Special Holy: %d%%", (int)sd->special_state.element_damage_rate_spe[ELE_HOLY] },
			{ "   -> Special Dark: %d%%", (int)sd->special_state.element_damage_rate_spe[ELE_DARK] },
			{ "   -> Special Undead: %d%%", (int)sd->special_state.element_damage_rate_spe[ELE_UNDEAD] },
		};
		if (ARRAYLENGTH(ele_bonus_damage_info) > 0)
			clif_displaymessage(fd, "|--- (Bonus max is +100%) ---|");
		for (int i = 0; i < ARRAYLENGTH(ele_bonus_damage_info); i++) {
			if (ele_bonus_damage_info[i].value == 0)
				continue;
			battlestats_msg(sprintf(output, ele_bonus_damage_info[i].format, ele_bonus_damage_info[i].value));
		}

		clif_displaymessage(fd, "|--- Damage on Elemental Targets ---|");
		strcpy(output, "");
		struct {
			const char* format;
			int value;
		} ele_damage_info[] = {
			{ "   -> Neutral: %d%% (right hand)", sd->right_weapon.addele[ELE_NEUTRAL] + sd->right_weapon.addele[ELE_ALL] },
			{ "   -> Water: %d%% (right hand)", sd->right_weapon.addele[ELE_WATER] + sd->right_weapon.addele[ELE_ALL] },
			{ "   -> Earth: %d%% (right hand)", sd->right_weapon.addele[ELE_EARTH] + sd->right_weapon.addele[ELE_ALL] },
			{ "   -> Fire: %d%% (right hand)", sd->right_weapon.addele[ELE_FIRE] + sd->right_weapon.addele[ELE_ALL] },
			{ "   -> Wind: %d%% (right hand)", sd->right_weapon.addele[ELE_WIND] + sd->right_weapon.addele[ELE_ALL] },
			{ "   -> Poison: %d%% (right hand)", sd->right_weapon.addele[ELE_POISON] + sd->right_weapon.addele[ELE_ALL] },
			{ "   -> Holy: %d%% (right hand)", sd->right_weapon.addele[ELE_HOLY] + sd->right_weapon.addele[ELE_ALL] },
			{ "   -> Dark: %d%% (right hand)", sd->right_weapon.addele[ELE_DARK] + sd->right_weapon.addele[ELE_ALL] },
			{ "   -> Ghost: %d%% (right hand)", sd->right_weapon.addele[ELE_GHOST] + sd->right_weapon.addele[ELE_ALL] },
			{ "   -> Undead: %d%% (right hand)", sd->right_weapon.addele[ELE_UNDEAD] + sd->right_weapon.addele[ELE_ALL] },
			{ "   -> Neutral: %d%% (left hand)", sd->left_weapon.addele[ELE_NEUTRAL] + sd->left_weapon.addele[ELE_ALL] },
			{ "   -> Water: %d%% (left hand)", sd->left_weapon.addele[ELE_WATER] + sd->left_weapon.addele[ELE_ALL] },
			{ "   -> Earth: %d%% (left hand)", sd->left_weapon.addele[ELE_EARTH] + sd->left_weapon.addele[ELE_ALL] },
			{ "   -> Fire: %d%% (left hand)", sd->left_weapon.addele[ELE_FIRE] + sd->left_weapon.addele[ELE_ALL] },
			{ "   -> Wind: %d%% (left hand)", sd->left_weapon.addele[ELE_WIND] + sd->left_weapon.addele[ELE_ALL] },
			{ "   -> Poison: %d%% (left hand)", sd->left_weapon.addele[ELE_POISON] + sd->left_weapon.addele[ELE_ALL] },
			{ "   -> Holy: %d%% (left hand)", sd->left_weapon.addele[ELE_HOLY] + sd->left_weapon.addele[ELE_ALL] },
			{ "   -> Dark: %d%% (left hand)", sd->left_weapon.addele[ELE_DARK] + sd->left_weapon.addele[ELE_ALL] },
			{ "   -> Ghost: %d%% (left hand)", sd->left_weapon.addele[ELE_GHOST] + sd->left_weapon.addele[ELE_ALL] },
			{ "   -> Undead: %d%% (left hand)", sd->left_weapon.addele[ELE_UNDEAD] + sd->left_weapon.addele[ELE_ALL] },
			{ "   -> Magic/Misc Water: %d%%", sd->indexed_bonus.magic_addele_script[ELE_WATER] + sd->indexed_bonus.magic_addele_script[ELE_ALL] },
			{ "   -> Magic/Misc Earth: %d%%", sd->indexed_bonus.magic_addele_script[ELE_EARTH] + sd->indexed_bonus.magic_addele_script[ELE_ALL] },
			{ "   -> Magic/Misc Fire: %d%%", sd->indexed_bonus.magic_addele_script[ELE_FIRE] + sd->indexed_bonus.magic_addele_script[ELE_ALL] },
			{ "   -> Magic/Misc Wind: %d%%", sd->indexed_bonus.magic_addele_script[ELE_WIND] + sd->indexed_bonus.magic_addele_script[ELE_ALL] },
			{ "   -> Magic/Misc Poison: %d%%", sd->indexed_bonus.magic_addele_script[ELE_POISON] + sd->indexed_bonus.magic_addele_script[ELE_ALL] },
			{ "   -> Magic/Misc Holy: %d%%", sd->indexed_bonus.magic_addele_script[ELE_HOLY] + sd->indexed_bonus.magic_addele_script[ELE_ALL] },
			{ "   -> Magic/Misc Dark: %d%%", sd->indexed_bonus.magic_addele_script[ELE_DARK] + sd->indexed_bonus.magic_addele_script[ELE_ALL] },
			{ "   -> Magic/Misc Ghost: %d%%", sd->indexed_bonus.magic_addele_script[ELE_GHOST] + sd->indexed_bonus.magic_addele_script[ELE_ALL] },
			{ "   -> Magic/Misc Undead: %d%%", sd->indexed_bonus.magic_addele_script[ELE_UNDEAD] + sd->indexed_bonus.magic_addele_script[ELE_ALL] },
			{ "   -> Coma Water: %d%%", sd->indexed_bonus.weapon_coma_ele[ELE_WATER] + sd->indexed_bonus.weapon_coma_ele[ELE_ALL] },
			{ "   -> Coma Earth: %d%%", sd->indexed_bonus.weapon_coma_ele[ELE_EARTH] + sd->indexed_bonus.weapon_coma_ele[ELE_ALL] },
			{ "   -> Coma Fire: %d%%", sd->indexed_bonus.weapon_coma_ele[ELE_FIRE] + sd->indexed_bonus.weapon_coma_ele[ELE_ALL] },
			{ "   -> Coma Wind: %d%%", sd->indexed_bonus.weapon_coma_ele[ELE_WIND] + sd->indexed_bonus.weapon_coma_ele[ELE_ALL] },
			{ "   -> Coma Poison: %d%%", sd->indexed_bonus.weapon_coma_ele[ELE_POISON] + sd->indexed_bonus.weapon_coma_ele[ELE_ALL] },
			{ "   -> Coma Holy: %d%%", sd->indexed_bonus.weapon_coma_ele[ELE_HOLY] + sd->indexed_bonus.weapon_coma_ele[ELE_ALL] },
			{ "   -> Coma Dark: %d%%", sd->indexed_bonus.weapon_coma_ele[ELE_DARK] + sd->indexed_bonus.weapon_coma_ele[ELE_ALL] },
			{ "   -> Coma Ghost: %d%%", sd->indexed_bonus.weapon_coma_ele[ELE_GHOST] + sd->indexed_bonus.weapon_coma_ele[ELE_ALL] },
			{ "   -> Coma Undead: %d%%", sd->indexed_bonus.weapon_coma_ele[ELE_UNDEAD] + sd->indexed_bonus.weapon_coma_ele[ELE_ALL] },
			{ "   -> Element Mastery Water: %d%%", (int)sd->special_state.element_mastery[ELE_NEUTRAL] + (int)sd->special_state.element_mastery[ELE_ALL] },
			{ "   -> Element Mastery Water: %d%%", (int)sd->special_state.element_mastery[ELE_WATER] + (int)sd->special_state.element_mastery[ELE_ALL] },
			{ "   -> Element Mastery Earth: %d%%", (int)sd->special_state.element_mastery[ELE_EARTH] + (int)sd->special_state.element_mastery[ELE_ALL] },
			{ "   -> Element Mastery Fire: %d%%", (int)sd->special_state.element_mastery[ELE_FIRE] + (int)sd->special_state.element_mastery[ELE_ALL] },
			{ "   -> Element Mastery Wind: %d%%", (int)sd->special_state.element_mastery[ELE_WIND] + (int)sd->special_state.element_mastery[ELE_ALL] },
			{ "   -> Element Mastery Poison: %d%%", (int)sd->special_state.element_mastery[ELE_POISON] + (int)sd->special_state.element_mastery[ELE_ALL] },
			{ "   -> Element Mastery Holy: %d%%", (int)sd->special_state.element_mastery[ELE_HOLY] + (int)sd->special_state.element_mastery[ELE_ALL] },
			{ "   -> Element Mastery Dark: %d%%", (int)sd->special_state.element_mastery[ELE_DARK] + (int)sd->special_state.element_mastery[ELE_ALL] },
			{ "   -> Element Mastery Ghost: %d%%", (int)sd->special_state.element_mastery[ELE_GHOST] + (int)sd->special_state.element_mastery[ELE_ALL] },
			{ "   -> Element Mastery Undead: %d%%", (int)sd->special_state.element_mastery[ELE_UNDEAD] + (int)sd->special_state.element_mastery[ELE_ALL] },
		};
		for (int i = 0; i < ARRAYLENGTH(ele_damage_info); i++) {
			if (ele_damage_info[i].value == 0)
				continue;
			battlestats_msg(sprintf(output, ele_damage_info[i].format, ele_damage_info[i].value));
		}

		clif_displaymessage(fd, "|--- Race Damage ---|");
		strcpy(output, "");
		struct {
			const char* format;
			int value;
		} race_damage_info[] = {
			{ "   -> Formless: %d%% (right hand)", sd->right_weapon.addrace[RC_FORMLESS] + sd->right_weapon.addrace[RC_ALL] },
			{ "   -> Undead: %d%% (right hand)", sd->right_weapon.addrace[RC_UNDEAD] + sd->right_weapon.addrace[RC_ALL] },
			{ "   -> Beast: %d%% (right hand)", sd->right_weapon.addrace[RC_BRUTE] + sd->right_weapon.addrace[RC_ALL] },
			{ "   -> Plant: %d%% (right hand)", sd->right_weapon.addrace[RC_PLANT] + sd->right_weapon.addrace[RC_ALL] },
			{ "   -> Insect: %d%% (right hand)", sd->right_weapon.addrace[RC_INSECT] + sd->right_weapon.addrace[RC_ALL] },
			{ "   -> Fish: %d%% (right hand)", sd->right_weapon.addrace[RC_FISH] + sd->right_weapon.addrace[RC_ALL] },
			{ "   -> Demon: %d%% (right hand)", sd->right_weapon.addrace[RC_DEMON] + sd->right_weapon.addrace[RC_ALL] },
			{ "   -> DemiHuman: %d%% (right hand)", sd->right_weapon.addrace[RC_DEMIHUMAN] + sd->right_weapon.addrace[RC_ALL] },
			{ "   -> Angel: %d%% (right hand)", sd->right_weapon.addrace[RC_ANGEL] + sd->right_weapon.addrace[RC_ALL] },
			{ "   -> Dragon: %d%% (right hand)", sd->right_weapon.addrace[RC_DRAGON] + sd->right_weapon.addrace[RC_ALL] },
			{ "   -> Player: %d%% (right hand)", sd->right_weapon.addrace[RC_PLAYER] + sd->right_weapon.addrace[RC_ALL] },
			{ "   -> Formless: %d%% (left hand)", sd->left_weapon.addrace[RC_FORMLESS] + sd->left_weapon.addrace[RC_ALL] },
			{ "   -> Undead: %d%% (left hand)", sd->left_weapon.addrace[RC_UNDEAD] + sd->left_weapon.addrace[RC_ALL] },
			{ "   -> Beast: %d%% (left hand)", sd->left_weapon.addrace[RC_BRUTE] + sd->left_weapon.addrace[RC_ALL] },
			{ "   -> Plant: %d%% (left hand)", sd->left_weapon.addrace[RC_PLANT] + sd->left_weapon.addrace[RC_ALL] },
			{ "   -> Insect: %d%% (left hand)", sd->left_weapon.addrace[RC_INSECT] + sd->left_weapon.addrace[RC_ALL] },
			{ "   -> Fish: %d%% (left hand)", sd->left_weapon.addrace[RC_FISH] + sd->left_weapon.addrace[RC_ALL] },
			{ "   -> Demon: %d%% (left hand)", sd->left_weapon.addrace[RC_DEMON] + sd->left_weapon.addrace[RC_ALL] },
			{ "   -> DemiHuman: %d%% (left hand)", sd->left_weapon.addrace[RC_DEMIHUMAN] + sd->left_weapon.addrace[RC_ALL] },
			{ "   -> Angel: %d%% (left hand)", sd->left_weapon.addrace[RC_ANGEL] + sd->left_weapon.addrace[RC_ALL] },
			{ "   -> Dragon: %d%% (left hand)", sd->left_weapon.addrace[RC_DRAGON] + sd->left_weapon.addrace[RC_ALL] },
			{ "   -> Player: %d%% (left hand)", sd->left_weapon.addrace[RC_PLAYER] + sd->left_weapon.addrace[RC_ALL] },
			{ "   -> Magic/Misc Formless: %d%%", sd->indexed_bonus.magic_addrace[RC_FORMLESS] + sd->indexed_bonus.magic_addrace[RC_ALL] },
			{ "   -> Magic/Misc Undead: %d%%", sd->indexed_bonus.magic_addrace[RC_UNDEAD] + sd->indexed_bonus.magic_addrace[RC_ALL] },
			{ "   -> Magic/Misc Beast: %d%%", sd->indexed_bonus.magic_addrace[RC_BRUTE] + sd->indexed_bonus.magic_addrace[RC_ALL] },
			{ "   -> Magic/Misc Plant: %d%%", sd->indexed_bonus.magic_addrace[RC_PLANT] + sd->indexed_bonus.magic_addrace[RC_ALL] },
			{ "   -> Magic/Misc Insect: %d%%", sd->indexed_bonus.magic_addrace[RC_INSECT] + sd->indexed_bonus.magic_addrace[RC_ALL] },
			{ "   -> Magic/Misc Fish: %d%%", sd->indexed_bonus.magic_addrace[RC_FISH] + sd->indexed_bonus.magic_addrace[RC_ALL] },
			{ "   -> Magic/Misc Demon: %d%%", sd->indexed_bonus.magic_addrace[RC_DEMON] + sd->indexed_bonus.magic_addrace[RC_ALL] },
			{ "   -> Magic/Misc DemiHuman: %d%%", sd->indexed_bonus.magic_addrace[RC_DEMIHUMAN] + sd->indexed_bonus.magic_addrace[RC_ALL] },
			{ "   -> Magic/Misc Angel: %d%%", sd->indexed_bonus.magic_addrace[RC_ANGEL] + sd->indexed_bonus.magic_addrace[RC_ALL] },
			{ "   -> Magic/Misc Dragon: %d%%", sd->indexed_bonus.magic_addrace[RC_DRAGON] + sd->indexed_bonus.magic_addrace[RC_ALL] },
			{ "   -> Magic/Misc Player: %d%%", sd->indexed_bonus.magic_addrace[RC_PLAYER] + sd->indexed_bonus.magic_addrace[RC_ALL] },
			{ "   -> Critical Formless: %d%%", sd->indexed_bonus.critaddrace[RC_FORMLESS] + sd->indexed_bonus.critaddrace[RC_ALL] },
			{ "   -> Critical Undead: %d%%", sd->indexed_bonus.critaddrace[RC_UNDEAD] + sd->indexed_bonus.critaddrace[RC_ALL] },
			{ "   -> Critical Beast: %d%%", sd->indexed_bonus.critaddrace[RC_BRUTE] + sd->indexed_bonus.critaddrace[RC_ALL] },
			{ "   -> Critical Plant: %d%%", sd->indexed_bonus.critaddrace[RC_PLANT] + sd->indexed_bonus.critaddrace[RC_ALL] },
			{ "   -> Critical Insect: %d%%", sd->indexed_bonus.critaddrace[RC_INSECT] + sd->indexed_bonus.critaddrace[RC_ALL] },
			{ "   -> Critical Fish: %d%%", sd->indexed_bonus.critaddrace[RC_FISH] + sd->indexed_bonus.critaddrace[RC_ALL] },
			{ "   -> Critical Demon: %d%%", sd->indexed_bonus.critaddrace[RC_DEMON] + sd->indexed_bonus.critaddrace[RC_ALL] },
			{ "   -> Critical DemiHuman: %d%%", sd->indexed_bonus.critaddrace[RC_DEMIHUMAN] + sd->indexed_bonus.critaddrace[RC_ALL] },
			{ "   -> Critical Angel: %d%%", sd->indexed_bonus.critaddrace[RC_ANGEL] + sd->indexed_bonus.critaddrace[RC_ALL] },
			{ "   -> Critical Dragon: %d%%", sd->indexed_bonus.critaddrace[RC_DRAGON] + sd->indexed_bonus.critaddrace[RC_ALL] },
			{ "   -> Critical Player: %d%%", sd->indexed_bonus.critaddrace[RC_PLAYER] + sd->indexed_bonus.critaddrace[RC_ALL] },
			{ "   -> Ignore MDEF Formless: %d%%", sd->indexed_bonus.ignore_mdef_by_race[RC_FORMLESS] + sd->indexed_bonus.ignore_mdef_by_race[RC_ALL] },
			{ "   -> Ignore MDEF Undead: %d%%", sd->indexed_bonus.ignore_mdef_by_race[RC_UNDEAD] + sd->indexed_bonus.ignore_mdef_by_race[RC_ALL] },
			{ "   -> Ignore MDEF Beast: %d%%", sd->indexed_bonus.ignore_mdef_by_race[RC_BRUTE] + sd->indexed_bonus.ignore_mdef_by_race[RC_ALL] },
			{ "   -> Ignore MDEF Plant: %d%%", sd->indexed_bonus.ignore_mdef_by_race[RC_PLANT] + sd->indexed_bonus.ignore_mdef_by_race[RC_ALL] },
			{ "   -> Ignore MDEF Insect: %d%%", sd->indexed_bonus.ignore_mdef_by_race[RC_INSECT] + sd->indexed_bonus.ignore_mdef_by_race[RC_ALL] },
			{ "   -> Ignore MDEF Fish: %d%%", sd->indexed_bonus.ignore_mdef_by_race[RC_FISH] + sd->indexed_bonus.ignore_mdef_by_race[RC_ALL] },
			{ "   -> Ignore MDEF Demon: %d%%", sd->indexed_bonus.ignore_mdef_by_race[RC_DEMON] + sd->indexed_bonus.ignore_mdef_by_race[RC_ALL] },
			{ "   -> Ignore MDEF DemiHuman: %d%%", sd->indexed_bonus.ignore_mdef_by_race[RC_DEMIHUMAN] + sd->indexed_bonus.ignore_mdef_by_race[RC_ALL] },
			{ "   -> Ignore MDEF Angel: %d%%", sd->indexed_bonus.ignore_mdef_by_race[RC_ANGEL] + sd->indexed_bonus.ignore_mdef_by_race[RC_ALL] },
			{ "   -> Ignore MDEF Dragon: %d%%", sd->indexed_bonus.ignore_mdef_by_race[RC_DRAGON] + sd->indexed_bonus.ignore_mdef_by_race[RC_ALL] },
			{ "   -> Ignore MDEF Player: %d%%", sd->indexed_bonus.ignore_mdef_by_race[RC_PLAYER] + sd->indexed_bonus.ignore_mdef_by_race[RC_ALL] },
			{ "   -> Ignore DEF Formless: %d%%", sd->indexed_bonus.ignore_def_by_race[RC_FORMLESS] + sd->indexed_bonus.ignore_def_by_race[RC_ALL] },
			{ "   -> Ignore DEF Undead: %d%%", sd->indexed_bonus.ignore_def_by_race[RC_UNDEAD] + sd->indexed_bonus.ignore_def_by_race[RC_ALL] },
			{ "   -> Ignore DEF Beast: %d%%", sd->indexed_bonus.ignore_def_by_race[RC_BRUTE] + sd->indexed_bonus.ignore_def_by_race[RC_ALL] },
			{ "   -> Ignore DEF Plant: %d%%", sd->indexed_bonus.ignore_def_by_race[RC_PLANT] + sd->indexed_bonus.ignore_def_by_race[RC_ALL] },
			{ "   -> Ignore DEF Insect: %d%%", sd->indexed_bonus.ignore_def_by_race[RC_INSECT] + sd->indexed_bonus.ignore_def_by_race[RC_ALL] },
			{ "   -> Ignore DEF Fish: %d%%", sd->indexed_bonus.ignore_def_by_race[RC_FISH] + sd->indexed_bonus.ignore_def_by_race[RC_ALL] },
			{ "   -> Ignore DEF Demon: %d%%", sd->indexed_bonus.ignore_def_by_race[RC_DEMON] + sd->indexed_bonus.ignore_def_by_race[RC_ALL] },
			{ "   -> Ignore DEF DemiHuman: %d%%", sd->indexed_bonus.ignore_def_by_race[RC_DEMIHUMAN] + sd->indexed_bonus.ignore_def_by_race[RC_ALL] },
			{ "   -> Ignore DEF Angel: %d%%", sd->indexed_bonus.ignore_def_by_race[RC_ANGEL] + sd->indexed_bonus.ignore_def_by_race[RC_ALL] },
			{ "   -> Ignore DEF Dragon: %d%%", sd->indexed_bonus.ignore_def_by_race[RC_DRAGON] + sd->indexed_bonus.ignore_def_by_race[RC_ALL] },
			{ "   -> Ignore DEF Player: %d%%", sd->indexed_bonus.ignore_def_by_race[RC_PLAYER] + sd->indexed_bonus.ignore_def_by_race[RC_ALL] },
			{ "   -> Coma Formless: %d%%", sd->indexed_bonus.weapon_coma_race[RC_FORMLESS] + sd->indexed_bonus.weapon_coma_race[RC_ALL] },
			{ "   -> Coma Undead: %d%%", sd->indexed_bonus.weapon_coma_race[RC_UNDEAD] + sd->indexed_bonus.weapon_coma_race[RC_ALL] },
			{ "   -> Coma Beast: %d%%", sd->indexed_bonus.weapon_coma_race[RC_BRUTE] + sd->indexed_bonus.weapon_coma_race[RC_ALL] },
			{ "   -> Coma Plant: %d%%", sd->indexed_bonus.weapon_coma_race[RC_PLANT] + sd->indexed_bonus.weapon_coma_race[RC_ALL] },
			{ "   -> Coma Insect: %d%%", sd->indexed_bonus.weapon_coma_race[RC_INSECT] + sd->indexed_bonus.weapon_coma_race[RC_ALL] },
			{ "   -> Coma Fish: %d%%", sd->indexed_bonus.weapon_coma_race[RC_FISH] + sd->indexed_bonus.weapon_coma_race[RC_ALL] },
			{ "   -> Coma Demon: %d%%", sd->indexed_bonus.weapon_coma_race[RC_DEMON] + sd->indexed_bonus.weapon_coma_race[RC_ALL] },
			{ "   -> Coma DemiHuman: %d%%", sd->indexed_bonus.weapon_coma_race[RC_DEMIHUMAN] + sd->indexed_bonus.weapon_coma_race[RC_ALL] },
			{ "   -> Coma Angel: %d%%", sd->indexed_bonus.weapon_coma_race[RC_ANGEL] + sd->indexed_bonus.weapon_coma_race[RC_ALL] },
			{ "   -> Coma Dragon: %d%%", sd->indexed_bonus.weapon_coma_race[RC_DRAGON] + sd->indexed_bonus.weapon_coma_race[RC_ALL] },
			{ "   -> Coma Player: %d%%", sd->indexed_bonus.weapon_coma_race[RC_PLAYER] + sd->indexed_bonus.weapon_coma_race[RC_ALL] },
		};
		for (int i = 0; i < ARRAYLENGTH(race_damage_info); i++) {
			if (race_damage_info[i].value == 0)
				continue;
			battlestats_msg(sprintf(output, race_damage_info[i].format, race_damage_info[i].value));
		}

		clif_displaymessage(fd, "|--- Size Damage ---|");
		strcpy(output, "");
		struct {
			const char* format;
			int value;
		} size_damage_info[] = {
			{ "   -> Small: %d%% (right hand)", sd->right_weapon.addsize[SZ_SMALL] + sd->right_weapon.addsize[SZ_ALL] },
			{ "   -> Medium: %d%% (right hand)", sd->right_weapon.addsize[SZ_MEDIUM] + sd->right_weapon.addsize[SZ_ALL] },
			{ "   -> Large: %d%% (right hand)", sd->right_weapon.addsize[SZ_BIG] + sd->right_weapon.addsize[SZ_ALL] },
			{ "   -> Small: %d%% (left hand)", sd->left_weapon.addsize[SZ_SMALL] + sd->left_weapon.addsize[SZ_ALL] },
			{ "   -> Medium: %d%% (left hand)", sd->left_weapon.addsize[SZ_MEDIUM] + sd->left_weapon.addsize[SZ_ALL] },
			{ "   -> Large: %d%% (left hand)", sd->left_weapon.addsize[SZ_BIG] + sd->left_weapon.addsize[SZ_ALL] },
			{ "   -> Magic/Misc Small: %d%%", sd->indexed_bonus.magic_addsize[SZ_SMALL] + sd->indexed_bonus.magic_addsize[SZ_ALL] },
			{ "   -> Magic/Misc Medium: %d%%", sd->indexed_bonus.magic_addsize[SZ_MEDIUM] + sd->indexed_bonus.magic_addsize[SZ_ALL] },
			{ "   -> Magic/Misc Large: %d%%", sd->indexed_bonus.magic_addsize[SZ_BIG] + sd->indexed_bonus.magic_addsize[SZ_ALL] },
		};
		for (int i = 0; i < ARRAYLENGTH(size_damage_info); i++) {
			if (size_damage_info[i].value == 0)
				continue;
			battlestats_msg(sprintf(output, size_damage_info[i].format, size_damage_info[i].value));
		}

		clif_displaymessage(fd, "|--- Class Damage ---|");
		strcpy(output, "");
		struct {
			const char* format;
			int value;
		} class_damage_info[] = {
			{ "   -> Normal: %d%% (right hand)", sd->right_weapon.addclass[CLASS_NORMAL] + sd->right_weapon.addclass[CLASS_ALL] },
			{ "   -> Boss: %d%% (right hand)", sd->right_weapon.addclass[CLASS_BOSS] + sd->right_weapon.addclass[CLASS_ALL] },
			{ "   -> Guardian: %d%% (right hand)", sd->right_weapon.addclass[CLASS_GUARDIAN] + sd->right_weapon.addclass[CLASS_ALL] },
			{ "   -> Normal: %d%% (left hand)", sd->left_weapon.addclass[CLASS_NORMAL] + sd->left_weapon.addclass[CLASS_ALL] },
			{ "   -> Boss: %d%% (left hand)", sd->left_weapon.addclass[CLASS_BOSS] + sd->left_weapon.addclass[CLASS_ALL] },
			{ "   -> Guardian: %d%% (left hand)", sd->left_weapon.addclass[CLASS_GUARDIAN] + sd->left_weapon.addclass[CLASS_ALL] },
			{ "   -> Magic/Misc Normal: %d%%", sd->indexed_bonus.magic_addclass[CLASS_NORMAL] + sd->indexed_bonus.magic_addclass[CLASS_ALL] },
			{ "   -> Magic/Misc Boss: %d%%", sd->indexed_bonus.magic_addclass[CLASS_BOSS] + sd->indexed_bonus.magic_addclass[CLASS_ALL] },
			{ "   -> Magic/Misc Guardian: %d%%", sd->indexed_bonus.magic_addclass[CLASS_GUARDIAN] + sd->indexed_bonus.magic_addclass[CLASS_ALL] },
			{ "   -> Ignore MDEF Normal: %d%%", sd->indexed_bonus.ignore_mdef_by_class[CLASS_NORMAL] + sd->indexed_bonus.ignore_mdef_by_class[CLASS_ALL] },
			{ "   -> Ignore MDEF Boss: %d%%", sd->indexed_bonus.ignore_mdef_by_class[CLASS_BOSS] + sd->indexed_bonus.ignore_mdef_by_class[CLASS_ALL] },
			{ "   -> Ignore MDEF Guardian: %d%%", sd->indexed_bonus.ignore_mdef_by_class[CLASS_GUARDIAN] + sd->indexed_bonus.ignore_mdef_by_class[CLASS_ALL] },
			{ "   -> Ignore DEF Normal: %d%%", sd->indexed_bonus.ignore_def_by_class[CLASS_NORMAL] + sd->indexed_bonus.ignore_def_by_class[CLASS_ALL] },
			{ "   -> Ignore DEF Boss: %d%%", sd->indexed_bonus.ignore_def_by_class[CLASS_BOSS] + sd->indexed_bonus.ignore_def_by_class[CLASS_ALL] },
			{ "   -> Ignore DEF Guardian: %d%%", sd->indexed_bonus.ignore_def_by_class[CLASS_GUARDIAN] + sd->indexed_bonus.ignore_def_by_class[CLASS_ALL] },
		};
		for (int i = 0; i < ARRAYLENGTH(class_damage_info); i++) {
			if (class_damage_info[i].value == 0)
				continue;
			battlestats_msg(sprintf(output, class_damage_info[i].format, class_damage_info[i].value));
		}

		clif_displaymessage(fd, "|--- Status Damage / Reflect ---|");
		strcpy(output, "");
		struct {
			const char* format;
			int value;
		} status_damage_info[] = {
			{ "   -> Bleeding: %d%%", (int)sd->special_state.status_damage_rate_all[SC_BLEEDING] + (int)sd->special_state.status_damage_rate_all[SC_NONE] },
			{ "   -> Blind: %d%%", (int)sd->special_state.status_damage_rate_all[SC_BLIND] + (int)sd->special_state.status_damage_rate_all[SC_NONE] },
			{ "   -> Burning: %d%%", (int)sd->special_state.status_damage_rate_all[SC_BURNING] + (int)sd->special_state.status_damage_rate_all[SC_NONE] },
			{ "   -> Confusion: %d%%", (int)sd->special_state.status_damage_rate_all[SC_CONFUSION] + (int)sd->special_state.status_damage_rate_all[SC_NONE] },
			{ "   -> Curse: %d%%", (int)sd->special_state.status_damage_rate_all[SC_CURSE] + (int)sd->special_state.status_damage_rate_all[SC_NONE] },
			{ "   -> DPoison: %d%%", (int)sd->special_state.status_damage_rate_all[SC_DPOISON] + (int)sd->special_state.status_damage_rate_all[SC_NONE] },
			{ "   -> Freeze: %d%%", (int)sd->special_state.status_damage_rate_all[SC_FREEZE] + (int)sd->special_state.status_damage_rate_all[SC_NONE] },
			{ "   -> Poison: %d%%", (int)sd->special_state.status_damage_rate_all[SC_POISON] + (int)sd->special_state.status_damage_rate_all[SC_NONE] },
			{ "   -> Silence: %d%%", (int)sd->special_state.status_damage_rate_all[SC_SILENCE] + (int)sd->special_state.status_damage_rate_all[SC_NONE] },
			{ "   -> Sleep: %d%%", (int)sd->special_state.status_damage_rate_all[SC_SLEEP] + (int)sd->special_state.status_damage_rate_all[SC_NONE] },
			{ "   -> Stone: %d%%", (int)sd->special_state.status_damage_rate_all[SC_STONE] + (int)sd->special_state.status_damage_rate_all[SC_NONE] },
			{ "   -> Stun: %d%%", (int)sd->special_state.status_damage_rate_all[SC_STUN] + (int)sd->special_state.status_damage_rate_all[SC_NONE] },
			{ "   -> Reflect Bleeding: %d%%", (int)sd->special_state.reflect_status[SC_BLEEDING] + (int)sd->special_state.reflect_status[SC_NONE] },
			{ "   -> Reflect Blind: %d%%", (int)sd->special_state.reflect_status[SC_BLIND] + (int)sd->special_state.reflect_status[SC_NONE] },
			{ "   -> Reflect Burning: %d%%", (int)sd->special_state.reflect_status[SC_BURNING] + (int)sd->special_state.reflect_status[SC_NONE] },
			{ "   -> Reflect Confusion: %d%%", (int)sd->special_state.reflect_status[SC_CONFUSION] + (int)sd->special_state.reflect_status[SC_NONE] },
			{ "   -> Reflect Curse: %d%%", (int)sd->special_state.reflect_status[SC_CURSE] + (int)sd->special_state.reflect_status[SC_NONE] },
			{ "   -> Reflect DPoison: %d%%", (int)sd->special_state.reflect_status[SC_DPOISON] + (int)sd->special_state.reflect_status[SC_NONE] },
			{ "   -> Reflect Freeze: %d%%", (int)sd->special_state.reflect_status[SC_FREEZE] + (int)sd->special_state.reflect_status[SC_NONE] },
			{ "   -> Reflect Poison: %d%%", (int)sd->special_state.reflect_status[SC_POISON] + (int)sd->special_state.reflect_status[SC_NONE] },
			{ "   -> Reflect Silence: %d%%", (int)sd->special_state.reflect_status[SC_SILENCE] + (int)sd->special_state.reflect_status[SC_NONE] },
			{ "   -> Reflect Sleep: %d%%", (int)sd->special_state.reflect_status[SC_SLEEP] + (int)sd->special_state.reflect_status[SC_NONE] },
			{ "   -> Reflect Stone: %d%%", (int)sd->special_state.reflect_status[SC_STONE] + (int)sd->special_state.reflect_status[SC_NONE] },
			{ "   -> Reflect Stun: %d%%", (int)sd->special_state.reflect_status[SC_STUN] + (int)sd->special_state.reflect_status[SC_NONE] },
		};
		for (int i = 0; i < ARRAYLENGTH(status_damage_info); i++) {
			if (status_damage_info[i].value == 0)
				continue;
			battlestats_msg(sprintf(output, status_damage_info[i].format, status_damage_info[i].value));
		}

		clif_displaymessage(fd, "|--- Status Related ---|");
		strcpy(output, "");
		int val1 = 0, val2 = 0, val3 = 0, val4 = 0, val5 = 0, val6 = 0, val7 = 0, val8 = 0, val9 = 0, val10 = 0, val11 = 0, val12 = 0;
		for (const auto& it : sd->addeff) {
			if (it.sc == SC_BLEEDING && !val1) { battlestats_msg(sprintf(output, "   -> Inflict Bleeding: %02.02f%%", (float)it.rate / 100)); val1++; }
			if (it.sc == SC_BLIND && !val2) { battlestats_msg(sprintf(output, "   -> Inflict Blind: %02.02f%%", (float)it.rate / 100)); val2++; }
			if (it.sc == SC_BURNING && !val3) { battlestats_msg(sprintf(output, "   -> Inflict Burning: %02.02f%%", (float)it.rate / 100)); val3++; }
			if (it.sc == SC_CONFUSION && !val4) { battlestats_msg(sprintf(output, "   -> Inflict Confusion: %02.02f%%", (float)it.rate / 100)); val4++; }
			if (it.sc == SC_CURSE && !val5) { battlestats_msg(sprintf(output, "   -> Inflict Curse: %02.02f%%", (float)it.rate / 100)); val5++; }
			if (it.sc == SC_DPOISON && !val6) { battlestats_msg(sprintf(output, "   -> Inflict Dpoison: %02.02f%%", (float)it.rate / 100)); val6++; }
			if (it.sc == SC_FREEZE && !val7) { battlestats_msg(sprintf(output, "   -> Inflict Freeze: %02.02f%%", (float)it.rate / 100)); val7++; }
			if (it.sc == SC_POISON && !val8) { battlestats_msg(sprintf(output, "   -> Inflict Poison: %02.02f%%", (float)it.rate / 100)); val8++; }
			if (it.sc == SC_SILENCE && !val9) { battlestats_msg(sprintf(output, "   -> Inflict Silence: %02.02f%%", (float)it.rate / 100)); val9++; }
			if (it.sc == SC_SLEEP && !val10) { battlestats_msg(sprintf(output, "   -> Inflict Sleep: %02.02f%%", (float)it.rate / 100)); val10++; }
			if (it.sc == SC_STONE && !val11) { battlestats_msg(sprintf(output, "   -> Inflict Stone: %02.02f%%", (float)it.rate / 100)); val11++; }
			if (it.sc == SC_STUN && !val12) { battlestats_msg(sprintf(output, "   -> Inflict Stun: %02.02f%%", (float)it.rate / 100)); val12++; }
		}

		clif_displaymessage(fd, "|--- Devotion to the Element ---|");
		strcpy(output, "");
		int water[19] = { MG_COLDBOLT,MG_FROSTDIVER,WZ_WATERBALL,WZ_ICEWALL,WZ_FROSTNOVA,WZ_STORMGUST,SA_FROSTWEAPON,SA_DELUGE,SA_ELEMENTWATER,NJ_HYOUSENSOU,NJ_SUITON,NJ_HYOUSYOURAKU,NPC_WATERATTACK,NPC_ICEBREATH,SU_SHIVA,SU_SHIVASK,SU_SHIVASK1,SU_SHIVASK2,SU_SHIVASK3 };
		int earth[19] = { MG_STONECURSE,WZ_EARTHSPIKE,WZ_HEAVENDRIVE,WZ_QUAGMIRE,SA_SEISMICWEAPON,SA_LANDPROTECTOR,SA_ELEMENTGROUND,HW_GRAVITATION,PF_FOGWALL,NPC_GROUNDATTACK,NPC_EARTHQUAKE,NPC_ACIDBREATH,WZ_EARTHWALL,SU_FENRIR,SU_FENRIRSK,SU_FENRIRSK1,SU_FENRIRSK2,SU_FENRIRSK3,SA_GAIA };
		int fire[20] = { MG_FIREBALL,MG_FIREWALL,MG_FIREBOLT,WZ_FIREPILLAR,WZ_SIGHTRASHER,WZ_METEOR,WZ_SIGHTBLASTER,SA_FLAMELAUNCHER,SA_VOLCANO,SA_ELEMENTFIRE,NJ_KOUENKA,NJ_KAENSIN,NJ_BAKUENRYU,NPC_FIREATTACK,NPC_FIREBREATH,SU_IFRIT,SU_IFRITSK,SU_IFRITSK1,SU_IFRITSK2,SU_IFRITSK3 };
		int wind[18] = { MG_LIGHTNINGBOLT,MG_THUNDERSTORM,WZ_JUPITEL,WZ_VERMILION,SA_LIGHTNINGLOADER,SA_VIOLENTGALE,SA_ELEMENTWIND,WZ_FIREIVY,NJ_HUUJIN,NJ_RAIGEKISAI,NJ_KAMAITACHI,NPC_WINDATTACK,NPC_THUNDERBREATH,SU_GOLGOTHA,SU_GOLGOTHASK,SU_GOLGOTHASK1,SU_GOLGOTHASK2,SU_GOLGOTHASK3 };
		int ghost[7] = { MG_SOULSTRIKE,MG_NAPALMBEAT,MG_SAFETYWALL,NPC_TELEKINESISATTACK,HW_NAPALMVULCAN,HW_MAGICCRASHER,HW_GRAVITATION };
		int devotionWater = 0, devotionEarth = 0, devotionFire = 0, devotionWind = 0, devotionGhost = 0;
		int i = 0;
		for (i = 0; i < ARRAYLENGTH(water); i++) {
			devotionWater += pc_checkskill(sd, water[i]) * 250;
			devotionEarth -= pc_checkskill(sd, water[i]) * 75;
			devotionFire -= pc_checkskill(sd, water[i]) * 75;
			devotionWind -= pc_checkskill(sd, water[i]) * 75;
		}
		for (i = 0; i < ARRAYLENGTH(earth); i++) {
			devotionWater -= pc_checkskill(sd, earth[i]) * 75;
			devotionEarth += pc_checkskill(sd, earth[i]) * 250;
			devotionFire -= pc_checkskill(sd, earth[i]) * 75;
			devotionWind -= pc_checkskill(sd, earth[i]) * 75;
		}
		for (i = 0; i < ARRAYLENGTH(fire); i++) {
			devotionWater -= pc_checkskill(sd, fire[i]) * 75;
			devotionEarth -= pc_checkskill(sd, fire[i]) * 75;
			devotionFire += pc_checkskill(sd, fire[i]) * 250;
			devotionWind -= pc_checkskill(sd, fire[i]) * 75;
		}
		for (i = 0; i < ARRAYLENGTH(wind); i++) {
			devotionWater -= pc_checkskill(sd, wind[i]) * 75;
			devotionEarth -= pc_checkskill(sd, wind[i]) * 75;
			devotionFire -= pc_checkskill(sd, wind[i]) * 75;
			devotionWind += pc_checkskill(sd, wind[i]) * 250;
		}
		for (i = 0; i < ARRAYLENGTH(ghost); i++)
			devotionGhost += pc_checkskill(sd, ghost[i]) * 250;
		if (devotionWater) battlestats_msg(sprintf(output, "   -> Devotion Water: %d%%", (devotionWater/100) > 100 ? 100 : (devotionWater/100)));
		if (devotionEarth) battlestats_msg(sprintf(output, "   -> Devotion Earth: %d%%", (devotionEarth/100) > 100 ? 100 : (devotionEarth/100)));
		if (devotionFire) battlestats_msg(sprintf(output, "   -> Devotion Fire: %d%%", (devotionFire/100) > 100 ? 100 : (devotionFire/100)));
		if (devotionWind) battlestats_msg(sprintf(output, "   -> Devotion Wind: %d%%", (devotionWind/100) > 100 ? 100 : (devotionWind/100)));
		if (devotionGhost) battlestats_msg(sprintf(output, "   -> Devotion Ghost: %d%%", (devotionGhost/100) > 100 ? 100 : (devotionGhost/100)));
	}
	strcpy(atcmd_output, " ");
	// Defensive Stats
	if(type == 2 || type == 0) {
		clif_displaymessage(fd, "|--- Elemental Resistance ---|");
		strcpy(output, "");
		struct {
			const char *format;
			int value;
		} ele_resist_info[] = {
			{ "   -> Neutral: %d%%", sd->indexed_bonus.subele[ELE_NEUTRAL] + sd->indexed_bonus.subele_script[ELE_NEUTRAL] + sd->indexed_bonus.subele_script[ELE_ALL] },
			{ "   -> Water: %d%%", sd->indexed_bonus.subele[ELE_WATER] + sd->indexed_bonus.subele_script[ELE_WATER] + sd->indexed_bonus.subele_script[ELE_ALL] },
			{ "   -> Earth: %d%%", sd->indexed_bonus.subele[ELE_EARTH] + sd->indexed_bonus.subele_script[ELE_EARTH] + sd->indexed_bonus.subele_script[ELE_ALL] },
			{ "   -> Fire: %d%%", sd->indexed_bonus.subele[ELE_FIRE] + sd->indexed_bonus.subele_script[ELE_FIRE] + sd->indexed_bonus.subele_script[ELE_ALL] },
			{ "   -> Wind: %d%%", sd->indexed_bonus.subele[ELE_WIND] + sd->indexed_bonus.subele_script[ELE_WIND] + sd->indexed_bonus.subele_script[ELE_ALL] },
			{ "   -> Poison: %d%%", sd->indexed_bonus.subele[ELE_POISON] + sd->indexed_bonus.subele_script[ELE_POISON] + sd->indexed_bonus.subele_script[ELE_ALL] },
			{ "   -> Holy: %d%%", sd->indexed_bonus.subele[ELE_HOLY] + sd->indexed_bonus.subele_script[ELE_HOLY] + sd->indexed_bonus.subele_script[ELE_ALL] },
			{ "   -> Dark: %d%%", sd->indexed_bonus.subele[ELE_DARK] + sd->indexed_bonus.subele_script[ELE_DARK] + sd->indexed_bonus.subele_script[ELE_ALL] },
			{ "   -> Ghost: %d%%", sd->indexed_bonus.subele[ELE_GHOST] + sd->indexed_bonus.subele_script[ELE_GHOST] + sd->indexed_bonus.subele_script[ELE_ALL] },
			{ "   -> Undead: %d%%", sd->indexed_bonus.subele[ELE_UNDEAD] + sd->indexed_bonus.subele_script[ELE_UNDEAD] + sd->indexed_bonus.subele_script[ELE_ALL] },
			{ "   -> Enemy Neutral Defense: %d%%", sd->indexed_bonus.subdefele[ELE_NEUTRAL] + sd->indexed_bonus.subdefele[ELE_ALL] },
			{ "   -> Enemy Water Defense: %d%%", sd->indexed_bonus.subdefele[ELE_WATER] + sd->indexed_bonus.subdefele[ELE_ALL] },
			{ "   -> Enemy Earth Defense: %d%%", sd->indexed_bonus.subdefele[ELE_EARTH] + sd->indexed_bonus.subdefele[ELE_ALL] },
			{ "   -> Enemy Fire Defense: %d%%", sd->indexed_bonus.subdefele[ELE_FIRE] + sd->indexed_bonus.subdefele[ELE_ALL] },
			{ "   -> Enemy Wind Defense: %d%%", sd->indexed_bonus.subdefele[ELE_WIND] + sd->indexed_bonus.subdefele[ELE_ALL] },
			{ "   -> Enemy Poison Defense: %d%%", sd->indexed_bonus.subdefele[ELE_POISON] + sd->indexed_bonus.subdefele[ELE_ALL] },
			{ "   -> Enemy Holy Defense: %d%%", sd->indexed_bonus.subdefele[ELE_HOLY] + sd->indexed_bonus.subdefele[ELE_ALL] },
			{ "   -> Enemy Dark Defense: %d%%", sd->indexed_bonus.subdefele[ELE_DARK] + sd->indexed_bonus.subdefele[ELE_ALL] },
			{ "   -> Enemy Ghost Defense: %d%%", sd->indexed_bonus.subdefele[ELE_GHOST] + sd->indexed_bonus.subdefele[ELE_ALL] },
			{ "   -> Enemy Undead Defense: %d%%", sd->indexed_bonus.subdefele[ELE_UNDEAD] + sd->indexed_bonus.subdefele[ELE_ALL] },
		};
		for (int i = 0; i < ARRAYLENGTH(ele_resist_info); i++) {
			if (ele_resist_info[i].value == 0)
				continue;
			battlestats_msg(sprintf(output, ele_resist_info[i].format, ele_resist_info[i].value));
		}

		clif_displaymessage(fd, "|--- Race Resistance ---|");
		strcpy(output, "");
		struct {
			const char *format;
			int value;
		} race_resist_info[] = {
			{ "   -> Formless: %d%%", sd->indexed_bonus.subrace[RC_FORMLESS] },
			{ "   -> Undead: %d%%", sd->indexed_bonus.subrace[RC_UNDEAD] },
			{ "   -> Beast: %d%%", sd->indexed_bonus.subrace[RC_BRUTE] },
			{ "   -> Plant: %d%%", sd->indexed_bonus.subrace[RC_PLANT] },
			{ "   -> Insect: %d%%", sd->indexed_bonus.subrace[RC_INSECT] },
			{ "   -> Fish: %d%%", sd->indexed_bonus.subrace[RC_FISH] },
			{ "   -> Demon: %d%%", sd->indexed_bonus.subrace[RC_DEMON] },
			{ "   -> DemiHuman: %d%%", sd->indexed_bonus.subrace[RC_DEMIHUMAN] },
			{ "   -> Angel: %d%%", sd->indexed_bonus.subrace[RC_ANGEL] },
			{ "   -> Dragon: %d%%", sd->indexed_bonus.subrace[RC_DRAGON] },
			{ "   -> Player: %d%%", sd->indexed_bonus.subrace[RC_PLAYER] },
			{ "   -> ALL: %d%%", sd->indexed_bonus.subrace[RC_ALL] },
			{ "   -> Guardian: %d%%", sd->indexed_bonus.subrace2[RC2_GUARDIAN] },
			{ "   -> Goblin: %d%%", sd->indexed_bonus.subrace2[RC2_GOBLIN] },
			{ "   -> Kobold: %d%%", sd->indexed_bonus.subrace2[RC2_KOBOLD] },
			{ "   -> Orc: %d%%", sd->indexed_bonus.subrace2[RC2_ORC] },
			{ "   -> Golem: %d%%", sd->indexed_bonus.subrace2[RC2_GOLEM] },
			{ "   -> Poring: %d%%", sd->indexed_bonus.subrace2[RC2_PORING] },
		};
		for (int i = 0; i < ARRAYLENGTH(race_resist_info); i++) {
			if (race_resist_info[i].value == 0)
				continue;
			battlestats_msg(sprintf(output, race_resist_info[i].format, race_resist_info[i].value));
		}

		clif_displaymessage(fd, "|--- Size Resistance ---|");
		strcpy(output, "");
		struct {
			const char *format;
			int value;
		} size_resist_info[] = {
			{ "   -> Small: %d%%", sd->indexed_bonus.subsize[SZ_SMALL] },
			{ "   -> Medium: %d%%", sd->indexed_bonus.subsize[SZ_MEDIUM] },
			{ "   -> Large: %d%%", sd->indexed_bonus.subsize[SZ_BIG] },
			{ "   -> All: %d%%", sd->indexed_bonus.subsize[SZ_ALL] },
		};
		for (int i = 0; i < ARRAYLENGTH(size_resist_info); i++) {
			if (size_resist_info[i].value == 0)
				continue;
			battlestats_msg(sprintf(output, size_resist_info[i].format, size_resist_info[i].value));
		}

		clif_displaymessage(fd, "|--- Class Resistance ---|");
		strcpy(output, "");
		struct {
			const char *format;
			int value;
		} class_resist_info[] = {
			{ "   -> Normal: %d%%", sd->indexed_bonus.subclass[CLASS_NORMAL] },
			{ "   -> Boss: %d%%", sd->indexed_bonus.subclass[CLASS_BOSS] },
			{ "   -> All: %d%%", sd->indexed_bonus.subclass[CLASS_ALL] },
		};
		for (int i = 0; i < ARRAYLENGTH(class_resist_info); i++) {
			if (class_resist_info[i].value == 0)
				continue;
			battlestats_msg(sprintf(output, class_resist_info[i].format, class_resist_info[i].value));
		}

		clif_displaymessage(fd, "|--- Resistance ---|");
		strcpy(output, "");
		struct {
			const char *format;
			int value;
		} resist_info[] = {
			{ "   -> Long Distance: %d%%", sd->bonus.long_attack_def_rate },
			{ "   -> Short Distance: %d%%", sd->bonus.near_attack_def_rate },
			{ "   -> Magical: %d%%", sd->bonus.magic_def_rate },
			{ "   -> Misc/Special: %d%%", sd->bonus.misc_def_rate },
		};
		for (int i = 0; i < ARRAYLENGTH(resist_info); i++) {
			if (resist_info[i].value == 0)
				continue;
			battlestats_msg(sprintf(output, resist_info[i].format, resist_info[i].value));
		}

		clif_displaymessage(fd, "|--- Status Related ---|");
		strcpy(output, "");
		int val1 = 0,val2 = 0,val3 = 0,val4 = 0,val5 = 0,val6 = 0,val7 = 0,val8 = 0,val9 = 0,val10 = 0,val11 = 0,val12 = 0;
		for (const auto &it : sd->reseff) {
			if(it.id == SC_BLEEDING && !val1) { battlestats_msg(sprintf(output, "   -> Resist Bleeding: %02.02f%%", (float)it.val / 100)); val1++; }
			if(it.id == SC_BLIND && !val2) { battlestats_msg(sprintf(output, "   -> Resist Blind: %02.02f%%", (float)it.val / 100)); val2++; }
			if(it.id == SC_BURNING && !val3) { battlestats_msg(sprintf(output, "   -> Resist Burning: %02.02f%%", (float)it.val / 100)); val3++; }
			if(it.id == SC_CONFUSION && !val4) { battlestats_msg(sprintf(output, "   -> Resist Confusion: %02.02f%%", (float)it.val / 100)); val4++; }
			if(it.id == SC_CURSE && !val5) { battlestats_msg(sprintf(output, "   -> Resist Curse: %02.02f%%", (float)it.val / 100)); val5++; }
			if(it.id == SC_DPOISON && !val6) { battlestats_msg(sprintf(output, "   -> Resist Dpoison: %02.02f%%", (float)it.val / 100)); val6++; }
			if(it.id == SC_FREEZE && !val7) { battlestats_msg(sprintf(output, "   -> Resist Freeze: %02.02f%%", (float)it.val / 100)); val7++; }
			if(it.id == SC_POISON && !val8) { battlestats_msg(sprintf(output, "   -> Resist Poison: %02.02f%%", (float)it.val / 100)); val8++; }
			if(it.id == SC_SILENCE && !val9) { battlestats_msg(sprintf(output, "   -> Resist Silence: %02.02f%%", (float)it.val / 100)); val9++; }
			if(it.id == SC_SLEEP && !val10) { battlestats_msg(sprintf(output, "   -> Resist Sleep: %02.02f%%", (float)it.val / 100)); val10++; }
			if(it.id == SC_STONE && !val11) { battlestats_msg(sprintf(output, "   -> Resist Stone: %02.02f%%", (float)it.val / 100)); val11++; }
			if(it.id == SC_STUN && !val12) { battlestats_msg(sprintf(output, "   -> Resist Stun: %02.02f%%", (float)it.val / 100)); val12++; }
		}
	}

#undef battlestats_msg
}

ACMD_FUNC(battlestats) {
	nullpo_retr(-1, sd);

	if (atoi(message) < 0 || atoi(message) > 4) {
		clif_displaymessage(fd, "Please enter a type (usage: @battlestats <type> )");
		clif_displaymessage(fd, "0 - All Bonuses");
		clif_displaymessage(fd, "1 - Offensive Bonuses");
		clif_displaymessage(fd, "2 - Defensive Bonuses");
		clif_displaymessage(fd, "3 - Utility Bonuses");
		clif_displaymessage(fd, "4 - Regeneration Bonuses");
		return -1;
	}

	atcommand_battlestats_sub(fd, sd, atoi(message));
	return 0;
}

#ifdef M5U00065
ACMD_FUNC(gemini)
{
	nullpo_retr(-1, sd);
	int type = atoi(message);
	if (!message || !*message || type != 1) {
		clif_displaymessage(fd, "Invalid Gemini Save Build. Type 1 to save your build");
		return -1;
	}
	if(pc_readglobalreg(sd, add_str("gemini") != 1)) {
		clif_displaymessage(fd, "You are not a Kind Gemini. Visit Reset NPC at Midgard's Castle.");
		return -1;
	}
	npc_event(sd, "AstralSkill::OnGemini", 0);
	return 0;
}
#endif
#ifdef M5U00059
ACMD_FUNC(hateffect)
{
	nullpo_retr(-1, sd);

	int16 effectID = atoi(message);

	if( effectID <= HAT_EF_MIN || effectID >= HAT_EF_MAX )
		return -1;

	sd->hatEffects.push_back( effectID );
	clif_hat_effect_single( sd, effectID, true );
	return 0;
}
ACMD_FUNC(resu)
{
	nullpo_retr(-1, sd);
	
	if( !pc_isdead(sd) ) {
		clif_displaymessage(fd, "Cannot use @resu if not dead.");
		return -1;
	}

	sd->hatEffects.push_back( 136 );
	clif_hat_effect_single( sd, 136, true );

	return 0;
}
ACMD_FUNC(noexp)
{
	nullpo_retr(-1, sd);

	if(sd->noexp) {
		sd->noexp = 0;
		clif_displaymessage(fd, "Stop EXP gain - OFF");
	} else {
		sd->noexp = 1;
		clif_displaymessage(fd, "Stop EXP gain - ON");
	}

	return 0;
}
#endif
#ifdef M5U00113
ACMD_FUNC(showpet)
{
	nullpo_retr(-1, sd);

	struct pet_data *pd = sd->pd;

	if(pd) {
		if(pd->vd.class_ == JT_INVISIBLE)
			pd->vd.class_ = pd->db->vd.class_;
		else
			pd->vd.class_ = JT_INVISIBLE;
		unit_refresh(&pd->bl);
	}
	return 0;
}
#endif
#ifdef M5U00116
ACMD_FUNC(combo)
{
	nullpo_retr(-1, sd);
	int type = atoi(message);
	if (!message || !*message || (type < 50 || type > 400)) {
		clif_displaymessage(fd, "Failed. Please choose a combo speed from 50 to 400 (0.05s and 0.4s)");
		return -1;
	}
	sd->combospeed = type;
	return 0;
}
#endif
#ifdef M5U00141
ACMD_FUNC(i)
{
	nullpo_retr(-1,sd);
	memset(atcmd_output, '\0', sizeof(atcmd_output));
	if( !message || !*message )
	{
		clif_displaymessage(fd, "Please, enter a message (usage: @i <message>).");
		return -1;
	}
	sprintf(atcmd_output, "%s: %s", sd->status.name, message);
	clif_broadcast2(&sd->bl, atcmd_output, (int)strlen(atcmd_output)+1, 0xFFFFFF, 0x190, 20, 0, 0, ALL_SAMEMAP);

	return 0;
}
#endif

#ifdef M5U00151
static TIMER_FUNC(dps_timer){
	struct block_list *bl = map_id2bl(id);
	struct map_session_data *sd = map_id2sd(id);
	if(sd) {
		char dps[50];
		sprintf( dps, "DPS: %d per second. (%d over %d seconds)", sd->dps/data,sd->dps,data );
		clif_displaymessage(sd->fd, dps);
		sd->dps = 0;
	}
	return 0;
}
ACMD_FUNC(dps)
{
	nullpo_retr(-1, sd);
	int type = atoi(message);
	if (!message || !*message || (type < 1 || type > 60)) {
		clif_displaymessage(fd, "Failed. Please enter a number between 1 ~ 60.");
		return -1;
	}
	if (sd->dps) {
		clif_displaymessage(fd, "Failed. You are currently doing a DPS test.");
		return -1;
	}
	clif_displaymessage(fd, "DPS analyses test started.");
	sd->dps = 1;
	add_timer(gettick()+type*1000,dps_timer,sd->bl.id,type);
	return 0;
}
#endif

#ifdef M5U00154
ACMD_FUNC(sumatk)
{
	nullpo_retr(-1, sd);
	if (sd->nosummon)
		clif_displaymessage(fd, "Summons are now targetable.");
	else
		clif_displaymessage(fd, "Summons are now not targetable.");
	pc_setglobalreg(sd, add_str("sumatk"), sd->nosummon ? 0 : 1);
	sd->nosummon = sd->nosummon ? 0 : 1;
	unit_refresh(&sd->bl, true);
	clif_refresh(sd);
	return 0;
}
#endif

ACMD_FUNC(agree)
{
	struct map_session_data *pl_sd;

	nullpo_retr(-1, sd);

	memset(atcmd_player_name, '\0', sizeof atcmd_player_name);

	if (!message || !*message || sscanf(message, "%23[^\n]", atcmd_player_name) < 1) {
		clif_displaymessage(fd, "Please enter a player name (usage: @enemystats <char name>)");
		return -1;
	}

	pl_sd = map_nick2sd(atcmd_player_name, true);
	if(pl_sd == nullptr) {
		clif_displaymessage(fd, msg_txt(sd, 3)); // Character not found.
		return -1;
	} else {
		mapreg_setregstr(add_str("$@partymembername$"),atcmd_player_name);
		npc_event(sd,"InstanceManager::OnAgree",0);
	}

	return 0;
}
