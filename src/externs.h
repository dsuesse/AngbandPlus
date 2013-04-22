/* File: externs.h */

/* Purpose: extern declarations (variables and functions) */

/*
 * Note that some files have their own header files
 * (z-virt.h, z-util.h, z-form.h, term.h, random.h)
 */


/*
 * Automatically generated "variable" declarations
 */

/* tables.c */
extern s16b ddd[9];
extern s16b ddx[10];
extern s16b ddy[10];
extern s16b ddx_ddd[9];
extern s16b ddy_ddd[9];
extern char hexsym[16];
extern byte adj_val_min[];
extern byte adj_val_max[];
extern byte adj_mag_study[];
extern byte adj_mag_mana[];
extern byte adj_mag_fail[];
extern byte adj_mag_stat[];
extern byte adj_chr_gold[];
extern byte adj_int_dev[];
extern byte adj_wis_sav[];
extern byte adj_dex_dis[];
extern byte adj_int_dis[];
extern byte adj_dex_ta[];
extern byte adj_str_td[];
extern byte adj_dex_th[];
extern byte adj_str_th[];
extern byte adj_str_wgt[];
extern byte adj_str_hold[];
extern byte adj_str_dig[];
extern byte adj_str_blow[];
extern byte adj_dex_blow[];
extern byte adj_dex_safe[];
extern byte adj_con_fix[];
extern byte adj_con_mhp[];
extern byte blows_table[12][12];
extern s16b arena_monsters[MAX_ARENA_MONS];
extern byte extract_energy[300];
extern s32b player_exp[PY_MAX_LEVEL];
extern player_sex sex_info[MAX_SEXES];
extern player_race race_info[MAX_RACES];
extern player_race_mod race_mod_info[MAX_RACE_MODS];
extern player_class class_info[MAX_CLASS];
extern player_magic magic_info[MAX_CLASS];
extern deity deity_info[MAX_GODS];
extern u32b fake_spell_flags[MAX_REALM][9][2];
extern u32b Mrealm_choices[MAX_CLASS];
extern u32b mrealm_choices[MAX_CLASS];
extern cptr realm_names[];
extern magic_type realm_info_base[MAX_REALM][64];
extern magic_type realm_info[MAX_REALM][64];
extern cptr spell_names[MAX_REALM][64][2];
extern cptr player_title[MAX_CLASS][PY_MAX_LEVEL/5];
extern cptr color_names[16];
extern cptr stat_names[6];
extern cptr stat_names_reduced[6];
extern cptr window_flag_desc[32];
extern option_type option_info[];
extern cptr chaos_patrons[MAX_PATRON];
extern int chaos_stats[MAX_PATRON];
extern int chaos_rewards[MAX_PATRON][20];
extern martial_arts ma_blows[MAX_MA];
extern magic_power mindcraft_powers[MAX_MINDCRAFT_POWERS];
extern magic_power necro_powers[MAX_NECRO_POWERS];
extern magic_power mimic_powers[MAX_MIMIC_POWERS];
extern alchemist_recipe alchemist_recipes[MAX_ALCHEMIST_RECIPES];
extern cptr deity_rarity[2];
extern cptr deity_niceness[10];
extern cptr deity_standing[11];
extern move_info_type move_info[9];
extern tactic_info_type tactic_info[9];
extern activation activation_info[MAX_T_ACT];
extern music music_info[MAX_MUSICS];
extern inscription_info_type inscription_info[MAX_INSCRIPTIONS];
extern cptr sense_desc[];
extern flags_group flags_groups[MAX_FLAG_GROUP];
extern power_type powers_type[POWER_MAX];
extern quest_type quest[MAX_Q_IDX];
extern cptr artifact_names_list;
extern monster_power monster_powers[96];
extern tval_desc tval_descs[];
extern between_exit between_exits[MAX_BETWEEN_EXITS];
extern int month_day[9];
extern cptr month_name[9];


/* variable.c */
extern cptr copyright[5];
extern byte version_major;
extern byte version_minor;
extern byte version_patch;
extern byte version_extra;
extern byte sf_major;
extern byte sf_minor;
extern byte sf_patch;
extern byte sf_extra;
extern u32b sf_xtra;
extern u32b sf_when;
extern u16b sf_lives;
extern u16b sf_saves;
extern u32b vernum; /* Version flag */
extern bool arg_fiddle;
extern bool arg_wizard;
extern bool arg_sound;
extern bool arg_graphics;
extern bool arg_force_original;
extern bool arg_force_roguelike;
extern bool character_generated;
extern bool character_dungeon;
extern bool character_loaded;
extern bool character_saved;
extern bool character_icky;
extern bool character_xtra;
extern u32b seed_flavor;
extern u32b seed_town;
extern u32b seed_dungeon;
extern s16b command_cmd;
extern s16b command_arg;
extern s16b command_rep;
extern s16b command_dir;
extern s16b command_see;
extern s16b command_gap;
extern s16b command_wrk;
extern s16b command_new;
extern s32b energy_use;
extern s16b choose_default;
extern bool create_up_stair;
extern bool create_down_stair;
extern bool create_up_shaft;
extern bool create_down_shaft;
extern bool msg_flag;
extern bool alive;
extern bool death;
extern s16b running;
extern s16b resting;
extern s16b cur_hgt;
extern s16b cur_wid;
extern s16b dun_level;
extern s16b old_dun_level;
extern s16b num_repro;
extern s16b object_level;
extern s16b monster_level;
extern s32b turn;
extern s32b old_turn;
extern bool wizard;
extern bool use_sound;
extern bool use_graphics;
extern u16b total_winner;
extern u16b panic_save;
extern u16b noscore;
extern s16b signal_count;
extern bool inkey_base;
extern bool inkey_xtra;
extern bool inkey_scan;
extern bool inkey_flag;
extern s16b coin_type;
extern bool opening_chest;
extern bool shimmer_monsters;
extern bool shimmer_objects;
extern bool repair_monsters;
extern bool repair_objects;
extern s16b inven_nxt;
extern s16b inven_cnt;
extern s16b equip_cnt;
extern s16b o_max;
extern s16b o_cnt;
extern s16b m_max;
extern s16b m_cnt;
extern s16b hack_m_idx;
extern s16b hack_m_idx_ii;
extern int total_friends;
extern s32b total_friend_levels;
extern int leaving_quest;
extern bool multi_rew;
extern char summon_kin_type;
extern bool hack_mind;
extern bool hack_corruption;
extern bool is_autosave;
extern int artifact_bias;
extern bool show_inven_graph;
extern bool show_store_graph;
extern bool show_equip_graph;
extern bool rogue_like_commands;
extern bool quick_messages;
extern bool other_query_flag;
extern bool carry_query_flag;
extern bool always_pickup;
extern bool prompt_pickup_heavy;
extern bool always_repeat;
extern bool use_old_target;
extern bool depth_in_feet;
extern bool use_color;
extern bool compress_savefile;
extern bool hilite_player;
extern bool ring_bell;
extern bool find_ignore_stairs;
extern bool find_ignore_doors;
extern bool find_cut;
extern bool find_examine;
extern bool disturb_near;
extern bool disturb_move;
extern bool disturb_panel;
extern bool disturb_state;
extern bool disturb_minor;
extern bool disturb_other;
extern bool avoid_abort;
extern bool avoid_other;
extern bool flush_disturb;
extern bool flush_failure;
extern bool flush_command;
extern bool fresh_before;
extern bool fresh_after;
extern bool fresh_message;
extern bool alert_hitpoint;
extern bool alert_failure;
extern bool view_yellow_lite;
extern bool view_bright_lite;
extern bool view_granite_lite;
extern bool view_special_lite;
extern bool plain_descriptions;
extern bool stupid_monsters;
extern bool auto_destroy;
extern bool wear_confirm;
extern bool confirm_stairs;
extern bool disturb_pets;
extern bool view_perma_grids;
extern bool view_torch_grids;
extern bool flow_by_sound;
extern bool flow_by_smell;
extern bool track_follow;
extern bool track_target;
extern bool stack_allow_items;
extern bool stack_allow_wands;
extern bool stack_force_notes;
extern bool stack_force_costs;
extern bool view_reduce_lite;
extern bool view_reduce_view;
extern bool auto_haggle;
extern bool auto_scum;
extern bool expand_look;
extern bool expand_list;
extern bool dungeon_align;
extern bool dungeon_stair;
extern bool smart_learn;
extern bool smart_cheat;
extern bool show_labels;
extern bool show_weights;
extern bool show_choices;
extern bool show_details;
extern bool testing_stack;
extern bool testing_carry;
extern bool cheat_peek;
extern bool cheat_hear;
extern bool cheat_room;
extern bool cheat_xtra;
extern bool cheat_know;
extern bool cheat_live;
extern bool last_words;              /* Zangband options */
extern bool speak_unique;
extern bool small_levels;
extern bool empty_levels;
extern bool always_small_level;
extern bool flavored_attacks;
extern bool player_symbols;
extern byte hitpoint_warn;
extern byte delay_factor;
extern s16b autosave_freq;
extern bool autosave_t;
extern bool autosave_l;
extern s16b feeling;
extern s16b rating;
extern bool good_item_flag;
extern bool closing_flag;
extern s16b max_panel_rows, max_panel_cols;
extern s16b panel_row_min, panel_row_max;
extern s16b panel_col_min, panel_col_max;
extern s16b panel_col_prt, panel_row_prt;
extern s16b py;
extern s16b px;
extern s16b target_who;
extern s16b target_col;
extern s16b target_row;
extern s16b health_who;
extern s16b monster_race_idx;
extern s16b monster_ego_idx;
extern s16b object_kind_idx;
extern int player_uid;
extern int player_euid;
extern int player_egid;
extern char player_name[32];
extern char player_base[32];
extern char died_from[80];
extern char history[4][60];
extern char savefile[1024];
extern s16b lite_n;
extern s16b lite_y[LITE_MAX];
extern s16b lite_x[LITE_MAX];
extern s16b view_n;
extern byte view_y[VIEW_MAX];
extern byte view_x[VIEW_MAX];
extern s16b temp_n;
extern byte temp_y[TEMP_MAX];
extern byte temp_x[TEMP_MAX];
extern s16b macro__num;
extern cptr *macro__pat;
extern cptr *macro__act;
extern bool *macro__cmd;
extern char *macro__buf;
extern s16b quark__num;
extern cptr *quark__str;
extern u16b message__next;
extern u16b message__last;
extern u16b message__head;
extern u16b message__tail;
extern u16b *message__ptr;
extern byte *message__color;
extern char *message__buf;
extern u32b option_flag[8];
extern u32b option_mask[8];
extern u32b window_flag[ANGBAND_TERM_MAX];
extern u32b window_mask[ANGBAND_TERM_MAX];
extern term *angband_term[ANGBAND_TERM_MAX];
extern char angband_term_name[ANGBAND_TERM_MAX][16];
extern byte angband_color_table[256][4];
extern char angband_sound_name[SOUND_MAX][16];
extern cave_type *cave[MAX_HGT];
extern object_type *o_list;
extern monster_type *m_list;
extern monster_type *km_list;
extern u16b max_real_towns;
extern u16b max_towns;
extern town_type *town;
extern object_type *inventory;
extern s16b alloc_kind_size;
extern alloc_entry *alloc_kind_table;
extern s16b alloc_race_size;
extern alloc_entry *alloc_race_table;
extern byte misc_to_attr[256];
extern char misc_to_char[256];
extern byte tval_to_attr[128];
extern char tval_to_char[128];
extern cptr keymap_act[KEYMAP_MODES][256];
extern player_type p_body;
extern player_type *p_ptr;
extern player_sex *sp_ptr;
extern player_race *rp_ptr;
extern player_race_mod *rmp_ptr;
extern player_class *cp_ptr;
extern player_magic *mp_ptr;
extern u32b spell_learned[MAX_REALM][2];
extern u32b spell_worked[MAX_REALM][2];
extern u32b spell_forgotten[MAX_REALM][2];
extern byte spell_order[64];
extern byte realm_order[64];
extern byte spell_level[MAX_REALM][64];
extern s16b player_hp[PY_MAX_LEVEL];
extern header *v_head;
extern vault_type *v_info;
extern char *v_name;
extern char *v_text;
extern header *f_head;
extern feature_type *f_info;
extern char *f_name;
extern char *f_text;
extern header *k_head;
extern object_kind *k_info;
extern char *k_name;
extern char *k_text;
extern header *a_head;
extern artifact_type *a_info;
extern char *a_name;
extern char *a_text;
extern header *e_head;
extern ego_item_type *e_info;
extern char *e_name;
extern char *e_text;
extern header *ra_head;
extern randart_part_type *ra_info;
extern header *r_head;
extern monster_race *r_info;
extern char *r_name;
extern char *r_text;
extern header *re_head;
extern monster_ego *re_info;
extern char *re_name;
extern header *d_head;
extern dungeon_info_type *d_info;
extern char *d_name;
extern char *d_text;
extern header *t_head;
extern trap_type *t_info;
extern char *t_name;
extern char *t_text;
extern header *wf_head;
extern wilderness_type_info *wf_info;
extern char *wf_name;
extern char *wf_text;
extern int wildc2i[256];
extern header *st_head;
extern store_info_type *st_info;
extern char *st_name;
extern header *ba_head;
extern store_action_type *ba_info;
extern char *ba_name;
extern header *ow_head;
extern owner_type *ow_info;
extern char *ow_name;
extern cptr ANGBAND_SYS;
extern cptr ANGBAND_GRAF;
extern cptr ANGBAND_DIR;
extern cptr ANGBAND_DIR_APEX;
extern cptr ANGBAND_DIR_BONE;
extern cptr ANGBAND_DIR_DNGN;
extern cptr ANGBAND_DIR_DATA;
extern cptr ANGBAND_DIR_EDIT;
extern cptr ANGBAND_DIR_FILE;
extern cptr ANGBAND_DIR_HELP;
extern cptr ANGBAND_DIR_INFO;
extern cptr ANGBAND_DIR_NOTE;
extern cptr ANGBAND_DIR_SAVE;
extern cptr ANGBAND_DIR_SCPT;
extern cptr ANGBAND_DIR_PREF;
extern cptr ANGBAND_DIR_USER;
extern cptr ANGBAND_DIR_XTRA;
extern cptr ANGBAND_DIR_CMOV;
extern bool item_tester_full;
extern byte item_tester_tval;
extern bool (*item_tester_hook)(object_type *o_ptr);
extern bool (*ang_sort_comp)(vptr u, vptr v, int a, int b);
extern void (*ang_sort_swap)(vptr u, vptr v, int a, int b);
extern bool (*get_mon_num_hook)(int r_idx);
extern bool (*get_mon_num2_hook)(int r_idx);
extern bool (*get_obj_num_hook)(int k_idx);
extern bool monk_armour_aux;
extern bool monk_notify_aux;
extern u16b max_wild_x;
extern u16b max_wild_y;
extern wilderness_map **wild_map;
extern u16b max_r_idx;
extern u16b max_re_idx;
extern u16b max_k_idx;
extern u16b max_v_idx;
extern u16b max_f_idx;
extern u16b max_a_idx;
extern u16b max_e_idx;
extern u16b max_ra_idx;
extern u16b max_d_idx;
extern u16b max_o_idx;
extern u16b max_m_idx;
extern u16b max_t_idx;
extern u16b max_st_idx;
extern u16b max_ba_idx;
extern u16b max_ow_idx;
extern u16b max_wf_idx;
extern int init_flags;
extern bool ambush_flag;
extern bool fate_flag;
extern u16b no_breeds;
extern bool carried_monster_hit;
extern random_artifact random_artifacts[MAX_RANDARTS];
extern s16b bounties[MAX_BOUNTIES][2];
extern random_spell random_spells[MAX_SPELLS];
extern s16b spell_num;
extern rune_spell rune_spells[MAX_RUNES];
extern s16b rune_num;
extern fate fates[MAX_FATES];
extern byte vanilla_town;
extern byte dungeon_type;
extern s16b *max_dlv;
extern u32b total_bounties;
extern s16b doppleganger;
extern bool generate_encounter;
extern bool permanent_levels;
extern bool autoroll;
extern bool point_based;
extern bool maximize, preserve, special_lvls, ironman_rooms;
extern bool take_notes, auto_notes;
extern bool *m_allow_special;
extern bool *k_allow_special;
extern bool *a_allow_special;
extern bool rand_birth;
extern bool fast_autoroller;
extern bool zang_monsters, joke_monsters, pern_monsters, cth_monsters;
extern bool munchkin_multipliers;
extern bool center_player;
#ifdef SAFER_PANICS
extern bool panicload;
#endif
extern bool astral_option;
extern s16b plots[MAX_PLOTS];
extern random_quest random_quests[MAX_RANDOM_QUEST];
extern bool exp_need;
extern bool autoload_old_colors;
extern bool fate_option;
extern bool *special_lvl[MAX_DUNGEON_DEPTH];
extern bool auto_more;
extern bool hack_map_info_default;
extern s32b dungeon_flags1;
extern birther previous_char;

/* plots.c */
extern FILE *hook_file;
extern s32b hook_option;
extern void init_hooks();
extern void add_hook(int h_idx, hook_type hook, cptr name);
extern void del_hook(int h_idx, hook_type hook);
extern bool process_hooks(int h_idx, int q_idx);

/* help.c */
extern void ingame_help(bool enable);

/* birth.c */
extern void save_savefile_names();
extern bool no_begin_screen;
extern bool begin_screen();
extern errr init_randart(void);
extern void player_birth(void);

/* cave.c */
extern int distance(int y1, int x1, int y2, int x2);
extern bool los(int y1, int x1, int y2, int x2);
extern bool player_can_see_bold(int y, int x);
extern bool cave_valid_bold(int y, int x);
extern bool no_lite(void);
#ifdef USE_TRANSPARENCY
extern void map_info(int y, int x, byte *ap, char *cp, byte *tap, char *tcp);
#else /* USE_TRANSPARENCY */
extern void map_info(int y, int x, byte *ap, char *cp);
#endif /* USE_TRANSPARENCY */
extern void move_cursor_relative(int row, int col);
extern void print_rel(char c, byte a, int y, int x);
extern void note_spot(int y, int x);
extern void lite_spot(int y, int x);
extern void prt_map(void);
extern void display_map(int *cy, int *cx);
extern void do_cmd_view_map(void);
extern void forget_lite(void);
extern void update_lite(void);
extern void forget_view(void);
extern void update_view(void);
extern void forget_flow(void);
extern void update_flow(void);
extern void map_area(void);
extern void wiz_lite(void);
extern void wiz_lite_extra(void);
extern void wiz_dark(void);
extern void cave_set_feat(int y, int x, int feat);
extern void mmove2(int *y, int *x, int y1, int x1, int y2, int x2);
extern bool projectable(int y1, int x1, int y2, int x2);
extern void scatter(int *yp, int *xp, int y, int x, int d, int m);
extern void health_track(int m_idx);
extern void monster_race_track(int r_idx, int ego);
extern void object_kind_track(int k_idx);
extern void disturb(int stop_search, int flush_output);
extern int is_quest(int level);
extern int random_quest_number();
extern void place_floor(int y, int x);

/* cmovie.c */
extern s16b do_play_cmovie(cptr cmov_file);
extern void do_record_cmovie(cptr cmovie);
extern void do_stop_cmovie();
extern void do_start_cmovie();

/* cmd1.c */
extern void attack_special(monster_type *m_ptr, byte special, int dam);
extern bool test_hit_fire(int chance, int ac, int vis);
extern bool test_hit_norm(int chance, int ac, int vis);
extern s16b critical_shot(int weight, int plus, int dam);
extern s16b critical_norm(int weight, int plus, int dam);
extern s16b tot_dam_aux(object_type *o_ptr, int tdam, monster_type *m_ptr, s32b *special);
extern void search(void);
extern void carry(int pickup);
extern void py_attack(int y, int x, int max_blow);
extern bool player_can_enter(byte feature);
extern void move_player(int dir, int do_pickup);
extern void run_step(int dir);
extern void step_effects(int y, int x, int do_pickup);
extern void do_cmd_pet(void);
extern bool do_cmd_integrate_body();
extern void do_cmd_leave_body(bool drop_body);
extern bool execute_inscription(byte i, byte y, byte x);
extern void do_cmd_engrave();
extern void do_spin();

/* cmd2.c */
extern int breakage_chance(object_type *o_ptr);
extern int throw_mult;
extern void do_cmd_go_up(void);
extern void do_cmd_go_down(void);
extern void do_cmd_search(void);
extern void do_cmd_toggle_search(void);
extern void do_cmd_open(void);
extern void do_cmd_close(void);
extern void do_cmd_give(void);
extern void do_cmd_tunnel(void);
extern bool do_cmd_tunnel_aux(int y, int x, int dir);
extern void do_cmd_disarm(void);
extern void do_cmd_bash(void);
extern void do_cmd_alter(void);
extern void do_cmd_spike(void);
extern void do_cmd_walk(int pickup);
extern void do_cmd_stay(int pickup);
extern void do_cmd_run(void);
extern void do_cmd_rest(void);
extern void do_cmd_fire(void);
extern void do_cmd_throw(void);
extern void do_cmd_boomerang(void);
extern void do_cmd_unwalk(void);
extern void do_cmd_immovable_special(void);
extern void fetch(int dir, int wgt, bool require_los);
extern void do_cmd_sacrifice(void);
extern void do_cmd_create_artifact();
extern void do_cmd_steal();
extern void do_cmd_racial_power(void);

/* cmd3.c */
extern void do_cmd_inven(void);
extern void do_cmd_equip(void);
extern void do_cmd_wield(void);
extern void do_cmd_takeoff(void);
extern void do_cmd_drop(void);
extern void do_cmd_destroy(void);
extern void do_cmd_observe(void);
extern void do_cmd_uninscribe(void);
extern void do_cmd_inscribe(void);
extern void do_cmd_refill(void);
extern void do_cmd_target(void);
extern void do_cmd_look(void);
extern void do_cmd_locate(void);
extern void do_cmd_query_symbol(void);
extern void do_cmd_sense_grid_mana();
extern bool research_mon(void);
extern s32b portable_hole_weight(void);
extern void set_portable_hole_weight(void);
extern void do_cmd_portable_hole(void);

/* cmd4.c */
extern void do_cmd_redraw(void);
extern void do_cmd_change_name(void);
extern void do_cmd_message_one(void);
extern void do_cmd_messages(void);
extern void do_cmd_options(void);
extern void do_cmd_pref(void);
extern void do_cmd_macros(void);
extern void do_cmd_visuals(void);
extern void do_cmd_colors(void);
extern void do_cmd_note(void);
extern void do_cmd_version(void);
extern void do_cmd_feeling(void);
extern void do_cmd_load_screen(void);
extern void do_cmd_save_screen(void);
extern void do_cmd_knowledge(void);
extern void plural_aux(char * Name);
extern void do_cmd_checkquest(void);
extern void do_cmd_change_tactic(int i);
extern void do_cmd_change_movement(int i);
extern void do_cmd_time(void);
extern void do_cmd_options_aux(int page, cptr info, bool read_only);


/* cmd5.c */
extern void cast_druid_spell(int spell, byte level);
extern void cast_sigaldry_spell(int spell, byte level);
extern void cast_crusade_spell(int spell, byte level);
extern void cast_tribal_spell(int spell, byte level);
extern void cast_spirit_spell(int spell, byte level);
extern void cast_nether_spell(int spell, byte level);
extern void cast_chaos_spell(int spell, byte level);
extern void cast_shadow_spell(int spell, byte level);
extern void cast_music_spell(int spell, byte level);
extern void cast_symbiotic_spell(int spell, byte level);
extern void cast_magery_spell(int spell, byte level);
extern void cast_valarin_spell(int spell, byte level);
extern byte get_spell_level(int realm, int spell);
extern long apply_power_dam(long dam, int lvl, byte power);
extern long apply_power_dice(long dice, int lvl, byte power);
extern long apply_power_dur(long dur, int lvl, byte power);
extern bool is_magestaff();
extern void calc_magic_bonus();
extern void do_cmd_browse_aux(object_type *o_ptr);
extern void do_cmd_browse(void);
extern void do_cmd_study(void);
extern void do_cmd_cast(void);
extern void do_cmd_pray(void);
extern void do_cmd_rerate(void);
extern void corrupt_player(void);
extern bool item_tester_hook_armour(object_type *o_ptr);
extern void fetch(int dir, int wgt, bool require_los);
extern void do_poly_self(void);
extern void brand_weapon(int brand_type);
extern int use_symbiotic_power(int r_idx, bool great, bool only_number, bool no_cost);
extern void cast_spell(int realm, int spell, byte level);

/* cmd6.c */
extern void do_cmd_eat_food(void);
extern void do_cmd_quaff_potion(void);
extern void do_cmd_read_scroll(void);
extern void do_cmd_aim_wand(void);
extern void do_cmd_use_staff(void);
extern void do_cmd_zap_rod(void);
extern void do_cmd_activate(void);
extern void do_cmd_rerate(void);
extern void do_cmd_cut_corpse(void);
extern void do_cmd_cure_meat(void);
extern void do_cmd_drink_fountain(void);
extern void do_cmd_fill_bottle(void);

/* cmd7.c */
extern int rune_exec(rune_spell *spell, int cost);
extern void necro_info(char *p, int power);
extern void mindcraft_info(char *p, int power);
extern random_spell* select_spell(bool quick);
extern void cast_magic_spell(int spell, byte level);
extern void cast_prayer_spell(int spell, byte level);
extern void cast_illusion_spell(int spell, byte level);
extern void do_cmd_mindcraft(void);
extern void do_cmd_mimic(void);
extern void do_cmd_alchemist(void);
extern void do_cmd_beastmaster(void);
extern void do_cmd_powermage(void);
extern void do_cmd_possessor();
extern void do_cmd_archer(void);
extern void do_cmd_necromancer(void);
extern void do_cmd_unbeliever(void);
extern void cast_daemon_spell(int spell, byte level);
extern void do_cmd_unbeliever();
extern void do_cmd_runecrafter(void);
extern s32b sroot(s32b n);


/* dungeon.c */
extern void play_game(bool new_game);
extern bool psychometry(void);

/* files.c */
extern void player_flags(u32b* f1, u32b* f2, u32b* f3, u32b* f4, u32b* f5, u32b* esp);
extern void wipe_saved();
extern void safe_setuid_drop(void);
extern void safe_setuid_grab(void);
extern s16b tokenize(char *buf, s16b num, char **tokens, char delim1, char delim2);
extern void display_player(int mode);
extern errr file_character(cptr name, bool full);
extern errr process_pref_file_aux(char *buf);
extern bool user_process_pref_file;
extern errr process_pref_file(cptr name);
extern errr check_time_init(void);
extern errr check_load_init(void);
extern errr check_time(void);
extern errr check_load(void);
extern void read_times(void);
extern bool show_file(cptr name, cptr what, int line, int mode);
extern void do_cmd_help(void);
extern void process_player_base();
extern void process_player_name(bool sf);
extern void get_name(void);
extern void do_cmd_suicide(void);
extern void do_cmd_save_game(void);
extern long total_points(void);
extern void display_scores(int from, int to);
extern void close_game(void);
extern void exit_game_panic(void);
extern void signals_ignore_tstp(void);
extern void signals_handle_tstp(void);
extern void signals_init(void);
extern errr get_rnd_line(char * file_name, char * output);
extern cptr get_line(char* fname, char* fdir, int line);
extern void do_cmd_knowledge_corruptions(void);
extern void race_legends(void);
extern void show_highclass(int building);
extern errr get_xtra_line(char * file_name, monster_type *m_ptr, char * output);


/* generate.c */
extern bool generate_fracave(int y0, int x0,int xsize,int ysize,int cutoff,bool light,bool room);
extern void generate_hmap(int y0, int x0,int xsiz,int ysiz,int grd,int roug,int cutoff);
extern void evolve_level(bool noise);
extern bool room_alloc(int x,int y,bool crowded,int by0,int bx0,int *xx,int *yy);
extern void generate_grid_mana();
extern byte calc_dungeon_type();
extern void generate_cave(void);
extern void plasma_recursive(int x1, int y1, int x2, int y2, int depth_max, int rough);
extern byte feat_wall_outer;
extern byte feat_wall_inner;
extern s16b floor_type[100], fill_type[100];
extern void build_rectangle(int y1, int x1, int y2, int x2, int feat, int info);


/* wild.c */
extern int generate_area(int y, int x, bool border, bool corner, bool refresh);
extern void wilderness_gen(int refresh);
extern void wilderness_gen_small();
extern void reveal_wilderness_around_player(int y, int x, int h, int w);
extern void town_gen(int t_idx);


/* init1.c */
extern int color_char_to_attr(char c);
extern errr init_v_info_txt(FILE *fp, char *buf, bool start);
extern errr init_f_info_txt(FILE *fp, char *buf);
extern errr init_k_info_txt(FILE *fp, char *buf);
extern errr init_a_info_txt(FILE *fp, char *buf);
extern errr init_ra_info_txt(FILE *fp, char *buf);
extern errr init_e_info_txt(FILE *fp, char *buf);
extern errr init_r_info_txt(FILE *fp, char *buf);
extern errr init_re_info_txt(FILE *fp, char *buf);
extern errr init_d_info_txt(FILE *fp, char *buf);
extern errr init_t_info_txt(FILE *fp, char *buf);
extern errr init_ba_info_txt(FILE *fp, char *buf);
extern errr init_st_info_txt(FILE *fp, char *buf);
extern errr init_ow_info_txt(FILE *fp, char *buf);
extern errr init_wf_info_txt(FILE *fp, char *buf);
extern bool process_dungeon_file_full;
extern errr process_dungeon_file(cptr name, int *yval, int *xval, int ymax, int xmax, bool init);

/* init2.c */
extern void create_stores_stock(int t);
extern errr init_v_info(void);
extern void init_file_paths(char *path);
extern void init_angband(void);
extern errr init_buildings(void);
#ifdef ALLOW_TEMPLATES
extern s16b error_idx;
extern s16b error_line;
extern u32b fake_name_size;
extern u32b fake_text_size;
#endif /* ALLOW_TEMPLATES */

/* loadsave.c */
extern s16b rd_variable();
extern void wr_variable(s16b *var);
extern void wr_scripts();
extern bool load_dungeon(char *ext);
extern errr rd_savefile_new(void);

/* melee1.c */
/* melee2.c */
extern int hack_x2, hack_y2;
extern bool mon_take_hit_mon(int s_idx, int m_idx, int dam, bool *fear, cptr note);
extern int check_hit2(int power, int level, int ac);
extern bool carried_make_attack_normal(int r_idx);
extern bool make_attack_normal(int m_idx, byte divis);
extern bool make_attack_spell(int m_idx);
extern void process_monsters(void);
extern void curse_equipment(int chance, int heavy_chance);
extern void curse_equipment_dg(int chance, int heavy_chance);

/* monster1.c */
extern void screen_roff(int r_idx, int ego, int remember);
extern void display_roff(int r_idx, int ego);

/* monster2.c */
extern bool mego_ok(int r_idx, int ego);
extern void monster_check_experience(int m_idx, bool silent);
extern void monster_gain_exp(int m_idx, u32b exp, bool silent);
extern monster_race* race_info_idx(int r_idx, int ego);
extern int get_wilderness_flag(void);
extern void sanity_blast(monster_type * m_ptr, bool necro);
extern void delete_monster_idx(int i);
extern void delete_monster(int y, int x);
extern void compact_monsters(int size);
extern void wipe_m_list(void);
extern s16b m_pop(void);
extern errr get_mon_num_prep(void);
extern s16b get_mon_num(int level);
extern void monster_desc(char *desc, monster_type *m_ptr, int mode);
extern void monster_race_desc(char *desc, int r_idx, int ego);
extern void lore_do_probe(int m_idx);
extern void lore_treasure(int m_idx, int num_item, int num_gold);
extern void update_mon(int m_idx, bool full);
extern void update_monsters(bool full);
extern bool place_monster_aux(int y, int x, int r_idx, bool slp, bool grp, int status);
extern bool place_monster(int y, int x, bool slp, bool grp);
extern bool alloc_horde(int y, int x);
extern bool alloc_monster(int dis, bool slp);
extern bool summon_specific(int y1, int x1, int lev, int type);
extern void monster_swap(int y1, int x1, int y2, int x2);
extern bool multiply_monster(int m_idx, bool charm, bool clone);
extern void update_smart_learn(int m_idx, int what);
extern bool summon_specific_friendly(int y1, int x1, int lev, int type, bool Group_ok);
extern s16b place_monster_one(int y, int x, int r_idx, int ego, bool slp, int status);
extern s16b player_place(int y, int x);
extern void monster_drop_carried_objects(monster_type *m_ptr);
extern bool monster_dungeon(int r_idx);
extern bool monster_quest(int r_idx);
extern bool monster_ocean(int r_idx);
extern bool monster_shore(int r_idx);
extern bool monster_town(int r_idx);
extern bool monster_wood(int r_idx);
extern bool monster_volcano(int r_idx);
extern bool monster_mountain(int r_idx);
extern bool monster_grass(int r_idx);
extern bool monster_deep_water(int r_idx);
extern bool monster_shallow_water(int r_idx);
extern bool monster_lava(int r_idx);
extern void set_mon_num_hook(void);
extern void set_mon_num2_hook(int y, int x);
extern bool monster_can_cross_terrain(byte feat, monster_race *r_ptr);
extern void corrupt_corrupted(void);

/* monster3.c */
extern bool can_create_companion();
extern void ai_deincarnate(int m_idx);
extern bool ai_possessor(int m_idx, int o_idx);
extern bool ai_multiply(int m_idx);
extern bool change_side(monster_type *m_ptr);
extern int is_friend(monster_type *m_ptr);
extern bool is_enemy(monster_type *m_ptr, monster_type *t_ptr);

/* object1.c */
/* object2.c */
extern s32b calc_total_weight(void);
extern void add_random_ego_flag(object_type *o_ptr, int fego, bool *limit_blows);
extern bool info_spell;
extern char spell_txt[50];
extern int grab_tval_desc(int tval, cptr *info, int i);
extern void init_match_theme(obj_theme theme);
extern bool hack_ignore_theme;
extern bool kind_is_legal(int k_idx);
extern bool check_book_realm(const int book_tval);
extern bool verify(cptr prompt, int item);
extern void flavor_init(void);
extern void reset_visuals(void);
extern int object_power(object_type *o_ptr);
extern void object_flags(object_type *o_ptr, u32b *f1, u32b *f2, u32b *f3, u32b *f4, u32b *f5, u32b *esp);
extern void object_flags_known(object_type *o_ptr, u32b *f1, u32b *f2, u32b *f3, u32b *f4, u32b *f5, u32b *esp);
extern void object_desc(char *buf, object_type *o_ptr, int pref, int mode);
extern void object_desc_store(char *buf, object_type *o_ptr, int pref, int mode);
extern bool identify_fully_aux(object_type *o_ptr, FILE *fff);
extern char index_to_label(int i);
extern s16b label_to_inven(int c);
extern s16b label_to_equip(int c);
extern s16b wield_slot(object_type *o_ptr);
extern cptr mention_use(int i);
extern cptr describe_use(int i);
extern void inven_item_charges(int item);
extern void inven_item_describe(int item);
extern void inven_item_increase(int item, int num);
extern bool inven_item_optimize(int item);
extern void floor_item_charges(int item);
extern void floor_item_describe(int item);
extern void floor_item_increase(int item, int num);
extern void floor_item_optimize(int item);
extern bool inven_carry_okay(object_type *o_ptr);
extern s16b inven_carry(object_type *o_ptr, bool final);
extern s16b inven_takeoff(int item, int amt, bool force_drop);
extern void inven_drop(int item, int amt, int dy, int dx, bool silent);
extern bool item_tester_okay(object_type *o_ptr);
extern void display_inven(void);
extern void display_equip(void);
extern void show_inven(void);
extern void show_equip(void);
extern void toggle_inven_equip(void);
extern bool get_item(int *cp, cptr pmt, cptr str, int mode);
extern void excise_object_idx(int o_idx);
extern void delete_object_idx(int o_idx);
extern void delete_object(int y, int x);
extern void compact_objects(int size);
extern void wipe_o_list(void);
extern s16b o_pop(void);
extern errr get_obj_num_prep(void);
extern s16b get_obj_num(int level);
extern void object_known(object_type *o_ptr);
extern void object_aware(object_type *o_ptr);
extern void object_tried(object_type *o_ptr);
extern s32b object_value(object_type *o_ptr);
extern s32b object_value_real(object_type *o_ptr);
extern bool object_similar(object_type *o_ptr, object_type *j_ptr);
extern void object_absorb(object_type *o_ptr, object_type *j_ptr);
extern s16b lookup_kind(int tval, int sval);
extern void object_wipe(object_type *o_ptr);
extern void object_prep(object_type *o_ptr, int k_idx);
extern void object_copy(object_type *o_ptr, object_type *j_ptr);
extern int hack_apply_magic_power;
extern void apply_magic(object_type *o_ptr, int lev, bool okay, bool good, bool great);
bool make_object(object_type *j_ptr, bool good, bool great, obj_theme theme);
extern void place_object(int y, int x, bool good, bool great);
extern bool make_gold(object_type *j_ptr);
extern void place_gold(int y, int x);
extern void process_objects(void);
extern s16b drop_near(object_type *o_ptr, int chance, int y, int x);
extern void acquirement(int y1, int x1, int num, bool great, bool known);
extern void pick_trap(int y, int x);
extern cptr item_activation(object_type *o_ptr,byte num);
extern void combine_pack(void);
extern void reorder_pack(void);
extern s16b spell_chance(int spell,int realm);
extern bool spell_okay(int spell, bool known, int realm);
extern int print_spells(byte *spells, int num, int y, int x, int realm);
extern void display_koff(int k_idx);
extern void random_artifact_resistance (object_type * o_ptr);
extern void random_resistance (object_type * o_ptr, bool is_scroll, int specific);
extern s16b floor_carry(int y, int x, object_type *j_ptr);
extern void pack_decay(int item);
extern void floor_decay(int item);
extern bool scan_floor(int *items, int *item_num, int y, int x, int mode);
extern void show_floor(int y, int x);
extern bool get_item_floor(int *cp, cptr pmt, cptr str, int mode);
extern void py_pickup_floor(int pickup);
extern s16b m_bonus(int max, int level);
extern void object_gain_level(object_type *o_ptr);
extern void gain_flag_group_flag(object_type *o_ptr, bool silent);
extern void gain_flag_group(object_type *o_ptr, bool silent);

/* powers.c */
extern void do_cmd_power();

/* traps.c */
extern bool player_activate_trap_type(s16b y, s16b x, object_type *i_ptr, s16b item);
extern void player_activate_door_trap(s16b y, s16b x);
extern void place_trap(int y, int x);
extern void place_trap_object(object_type *o_ptr);
extern void wiz_place_trap(int y, int x, int idx);
extern void do_cmd_set_trap(void);
extern bool mon_hit_trap(int);

/* save.c */
extern void wr_s16b(s16b v);
extern void wr_string(cptr str);
extern void save_dungeon(void);
extern bool save_player(void);
extern bool load_player(void);

/* spells1.c */
extern s16b poly_r_idx(int r_idx);
extern void get_pos_player(int dis, int *ny, int *nx);
extern void teleport_to_player(int m_idx);
extern void teleport_player_directed(int rad, int dir);
extern void teleport_away(int m_idx, int dis);
extern void teleport_player(int dis);
extern void teleport_player_to(int ny, int nx);
extern void teleport_monster_to(int m_idx, int ny, int nx);
extern void teleport_player_level(void);
extern void recall_player(void);
extern void take_hit(int damage, cptr kb_str);
extern void take_sanity_hit(int damage, cptr hit_from);
extern void acid_dam(int dam, cptr kb_str);
extern void elec_dam(int dam, cptr kb_str);
extern void fire_dam(int dam, cptr kb_str);
extern void cold_dam(int dam, cptr kb_str);
extern bool inc_stat(int stat);
extern bool dec_stat(int stat, int amount, int mode);
extern bool res_stat(int stat);
extern bool apply_disenchant(int mode);
extern bool project_m(int who, int r, int y, int x, int dam, int typ);
extern sint project_path(u16b *gp, int range, int y1, int x1, int y2, int x2, int flg);
extern bool project(int who, int rad, int y, int x, int dam, int typ, int flg);
extern bool potion_smash_effect(int who, int y, int x, int o_sval);
extern void do_poly_self(void);
extern void corrupt_player(void);
extern void generate_spell(int plev);
extern bool unsafe;
extern void describe_attack_fully(int type, char* r);


/* spells2.c */
extern void curse_artifact(object_type * o_ptr);
extern void build_prob(cptr learn);
extern void grow_trees(int rad);
extern bool hp_player(int num);
extern bool heal_insanity(int val);
extern void warding_glyph(void);
extern void explosive_rune(void);
extern bool do_dec_stat(int stat, int mode);
extern bool do_res_stat(int stat);
extern bool do_inc_stat(int stat);
extern void identify_pack(void);
extern void message_pain(int m_idx, int dam);
extern bool remove_curse(void);
extern bool remove_all_curse(void);
extern bool restore_level(void);
extern void self_knowledge(FILE *fff);
extern bool lose_all_info(void);
extern bool detect_traps(void);
extern bool detect_doors(void);
extern bool detect_stairs(void);
extern bool detect_treasure(void);
extern bool hack_no_detect_message;
extern bool detect_objects_gold(void);
extern bool detect_objects_normal(void);
extern bool detect_objects_magic(void);
extern bool detect_monsters_normal(void);
extern bool detect_monsters_invis(void);
extern bool detect_monsters_evil(void);
extern bool detect_monsters_good(void);
extern bool detect_monsters_xxx(u32b match_flag);
extern bool detect_monsters_string(cptr);
extern bool detect_monsters_nonliving(void);
extern bool detect_all(void);
extern void stair_creation(void);
extern bool wall_stone(void);
extern bool enchant(object_type *o_ptr, int n, int eflag);
extern bool enchant_spell(int num_hit, int num_dam, int num_ac, int num_pval);
extern bool create_artifact(object_type *o_ptr, bool a_scroll, bool get_name);
extern bool artifact_scroll(void);
extern bool ident_spell(void);
extern bool identify_fully(void);
extern bool recharge(int num);
extern bool speed_monsters(void);
extern bool slow_monsters(void);
extern bool sleep_monsters(void);
extern bool conf_monsters(void);
extern void aggravate_monsters(int who);
extern bool genocide_aux(bool player_cast, char typ);
extern bool genocide(bool player_cast);
extern bool mass_genocide(bool player_cast);
extern void do_probe(int m_idx);
extern bool probing(void);
extern void change_wild_mode(void);
extern bool banish_evil(int dist);
extern bool dispel_evil(int dam);
extern bool dispel_good(int dam);
extern bool dispel_undead(int dam);
extern bool dispel_monsters(int dam);
extern bool dispel_living(int dam);
extern bool dispel_demons(int dam);
extern bool turn_undead(void);
extern void wipe(int y1, int x1, int r);
extern void destroy_area(int y1, int x1, int r, bool full);
extern void earthquake(int cy, int cx, int r);
extern void lite_room(int y1, int x1);
extern void unlite_room(int y1, int x1);
extern bool lite_area(int dam, int rad);
extern bool unlite_area(int dam, int rad);
extern bool fire_ball_beam(int typ, int dir, int dam, int rad);
extern bool fire_ball(int typ, int dir, int dam, int rad);
extern bool fire_bolt(int typ, int dir, int dam);
extern bool fire_beam(int typ, int dir, int dam);
extern bool fire_druid_ball(int typ, int dir, int dam, int rad);
extern bool fire_druid_bolt(int typ, int dir, int dam);
extern bool fire_druid_beam(int typ, int dir, int dam);
extern void call_chaos(void);
extern bool fire_bolt_or_beam(int prob, int typ, int dir, int dam);
extern bool lite_line(int dir);
extern bool drain_life(int dir, int dam);
extern bool death_ray(int dir, int plev);
extern bool wall_to_mud(int dir);
extern bool destroy_door(int dir);
extern bool disarm_trap(int dir);
extern bool wizard_lock(int dir);
extern bool heal_monster(int dir);
extern bool speed_monster(int dir);
extern bool slow_monster(int dir);
extern bool sleep_monster(int dir);
extern bool stasis_monster(int dir);    /* Like sleep, affects undead as well */
extern bool confuse_monster(int dir, int plev);
extern bool stun_monster(int dir, int plev);
extern bool fear_monster(int dir, int plev);
extern bool scare_monsters(void);
extern bool poly_monster(int dir);
extern bool clone_monster(int dir);
extern bool teleport_monster(int dir);
extern bool door_creation(void);
extern bool trap_creation(void);
extern bool glyph_creation(void);
extern bool destroy_doors_touch(void);
extern bool sleep_monsters_touch(void);
extern bool alchemy(void);
extern void activate_ty_curse(void);
extern void activate_dg_curse(void);
extern void activate_hi_summon(void);
extern void summon_cyber(void);
extern void wall_breaker(void);
extern void bless_weapon(void);
extern bool confuse_monsters(int dam);
extern bool charm_monsters(int dam);
extern bool charm_animals(int dam);
extern bool charm_demons(int dam);
extern bool stun_monsters(int dam);
extern bool stasis_monsters(int dam);
extern bool banish_monsters(int dist);
extern bool turn_monsters(int dam);
extern bool turn_evil(int dam);
extern bool deathray_monsters(void);
extern bool charm_monster(int dir, int plev);
extern bool star_charm_monster(int dir, int plev);
extern bool control_one_undead(int dir, int plev);
extern bool charm_animal(int dir, int plev);
extern bool mindblast_monsters(int dam);
extern void get_table_name(char * out_string);
extern s32b flag_cost(object_type * o_ptr, int plusses);
extern void alter_reality(void);
extern void report_magics(void);
extern void teleport_swap(int dir);
extern void swap_position(int lty, int ltx);
extern bool item_tester_hook_recharge(object_type *o_ptr);
extern bool fire_explosion(int y, int x, int typ, int rad, int dam);
extern bool fire_godly_wrath(int y, int x, int typ, int dir, int dam);
extern int  get_activation_power();
extern bool invoke(int dam, int typ);
extern bool project_hack(int typ, int dam);
extern void project_meteor(int radius, int typ, int dam, u32b flg);
extern bool item_tester_hook_artifactable(object_type *o_ptr);
extern bool passwall(int dir, bool safe);
extern bool project_hook(int typ, int dir, int dam, int flg);
extern void random_misc (object_type * o_ptr, bool is_scroll);
extern void random_plus(object_type * o_ptr, bool is_scroll);
extern void random_slay (object_type * o_ptr, bool is_scroll);
extern bool reset_recall(void);
extern void remove_dg_curse();

/* store.c */
extern void do_cmd_store(void);
extern void store_shuffle(int which);
extern void store_maint(int town_num, int store_num);
extern void store_init(int town_num, int store_num);
extern void move_to_black_market(object_type * o_ptr);
extern void do_cmd_home_trump(void);
extern void store_sell(void);
extern void store_purchase(void);
extern void store_examine(void);
extern void store_stole(void);
extern void store_prt_gold(void);
extern void store_request_item(void);

/* bldg.c -KMW- */
extern bool bldg_process_command(store_type *s_ptr, int i);
extern void show_building(store_type *s_ptr);
extern bool is_state(store_type *s_ptr, int state);
extern void do_cmd_bldg(void);
extern bool show_god_info(bool ext);
extern void do_cmd_quest(void);
extern void select_bounties(void);

/* util.c */
extern cptr get_month_name(int month, bool full, bool compact);
extern cptr get_day(int day);
extern s32b bst(s32b what, s32b t);
extern errr path_parse(char *buf, int max, cptr file);
extern errr path_temp(char *buf, int max);
extern errr path_build(char *buf, int max, cptr path, cptr file);
extern FILE *my_fopen(cptr file, cptr mode);
extern errr my_fgets(FILE *fff, char *buf, huge n);
extern errr my_fputs(FILE *fff, cptr buf, huge n);
extern errr my_fclose(FILE *fff);
extern errr fd_kill(cptr file);
extern errr fd_move(cptr file, cptr what);
extern errr fd_copy(cptr file, cptr what);
extern int fd_make(cptr file, int mode);
extern int fd_open(cptr file, int flags);
extern errr fd_lock(int fd, int what);
extern errr fd_seek(int fd, huge n);
extern errr fd_chop(int fd, huge n);
extern errr fd_read(int fd, char *buf, huge n);
extern errr fd_write(int fd, cptr buf, huge n);
extern errr fd_close(int fd);
extern void flush(void);
extern void bell(void);
extern void sound(int num);
extern void move_cursor(int row, int col);
extern void text_to_ascii(char *buf, cptr str);
extern void ascii_to_text(char *buf, cptr str);
extern void keymap_init(void);
extern errr macro_add(cptr pat, cptr act);
extern sint macro_find_exact(cptr pat);
extern char inkey(void);
extern cptr quark_str(s16b num);
extern s16b quark_add(cptr str);
extern s16b message_num(void);
extern cptr message_str(int age);
extern byte message_color(int age);
extern void message_add(cptr msg, byte color);
extern void cmsg_print(byte color, cptr msg);
extern void msg_print(cptr msg);
extern void cmsg_format(byte color, cptr fmt, ...);
extern void msg_format(cptr fmt, ...);
extern void screen_save(void);
extern void screen_load(void);
extern void c_put_str(byte attr, cptr str, int row, int col);
extern void put_str(cptr str, int row, int col);
extern void c_prt(byte attr, cptr str, int row, int col);
extern void prt(cptr str, int row, int col);
extern void c_roff(byte attr, cptr str);
extern void roff(cptr str);
extern void clear_screen(void);
extern void clear_from(int row);
extern bool askfor_aux(char *buf, int len);
extern bool get_string(cptr prompt, char *buf, int len);
extern bool get_check(cptr prompt);
extern bool get_com(cptr prompt, char *command);
extern s32b get_quantity(cptr prompt, s32b max);
extern void pause_line(int row);
extern void request_command(int shopping);
extern bool is_a_vowel(int ch);
extern int get_keymap_dir(char ch);
extern byte count_bits(u32b array);
extern void strlower(char *buf);
extern int test_monster_name(cptr name);
extern int test_item_name(cptr name);

/* xtra1.c */
extern int weight_limit(void);
extern bool calc_powers_silent;
extern void cnv_stat(int val, char *out_val);
extern s16b modify_stat_value(int value, int amount);
extern void calc_hitpoints(void);
extern void notice_stuff(void);
extern void update_stuff(void);
extern void redraw_stuff(void);
extern void window_stuff(void);
extern void handle_stuff(void);
extern bool monk_empty_hands(void);
extern bool monk_heavy_armor(void);
extern bool beastmaster_whip();
extern void display_spell_list(void);
extern void calc_bonuses(void);
extern void calc_sanity(void);
extern void gain_fate(byte fate);
extern void fate_desc(char *desc, int fate);
extern void dump_fates(FILE *OutFile);

/* xtra2.c */
extern void clean_wish_name(char *buf, char *name);
extern bool test_object_wish(char *name, object_type *o_ptr, object_type *forge, char *what);
extern bool set_parasite(int v, int r);
extern bool set_disrupt_shield(int v);
extern bool set_prob_travel(int v);
extern bool set_tim_deadly(int v);
extern bool set_tim_res_time(int v);
extern bool set_tim_reflect(int v);
extern bool set_meditation(int v);
extern bool set_strike(int v);
extern bool set_walk_water(int v);
extern bool set_tim_ffall(int v);
extern bool set_tim_fire_aura(int v);
extern bool set_holy(int v);
extern void set_grace(s32b v);
extern bool set_mimic(int v, int p);
extern bool set_no_breeders(int v);
extern bool set_invis(int v,int p);
extern bool set_lite(int v);
extern bool set_blind(int v);
extern bool set_confused(int v);
extern bool set_poisoned(int v);
extern bool set_afraid(int v);
extern bool set_paralyzed(int v);
extern bool set_image(int v);
extern bool set_fast(int v);
extern bool set_light_speed(int v);
extern bool set_slow(int v);
extern bool set_shield(int v, int p, s16b o);
extern bool set_blessed(int v);
extern bool set_hero(int v);
extern bool set_shero(int v);
extern bool set_protevil(int v);
extern bool set_protgood(int v);
extern bool set_protundead(int v);
extern bool set_invuln(int v);
extern bool set_tim_invis(int v);
extern bool set_tim_infra(int v);
extern bool set_mental_barrier(int v);
extern bool set_oppose_acid(int v);
extern bool set_oppose_elec(int v);
extern bool set_oppose_fire(int v);
extern bool set_oppose_cold(int v);
extern bool set_oppose_pois(int v);
extern bool set_oppose_ld(int v);
extern bool set_oppose_cc(int v);
extern bool set_oppose_ss(int v);
extern bool set_oppose_nex(int v);
extern bool set_stun(int v);
extern bool set_cut(int v);
extern bool set_food(int v);
extern void check_experience(void);
extern void check_experience_obj(object_type *o_ptr);
extern void gain_exp(s32b amount);
extern void lose_exp(s32b amount);
extern int get_coin_type(monster_race *r_ptr);
extern void monster_death(int m_idx);
extern bool mon_take_hit(int m_idx, int dam, bool *fear, cptr note);
extern void panel_bounds(void);
extern void verify_panel(void);
extern cptr look_mon_desc(int m_idx);
extern void ang_sort_aux(vptr u, vptr v, int p, int q);
extern void ang_sort(vptr u, vptr v, int n);
extern bool target_able(int m_idx);
extern bool target_okay(void);
extern bool target_set(int mode);
extern bool get_aim_dir(int *dp);
extern bool get_hack_dir(int *dp);
extern bool get_rep_dir(int *dp);
extern int get_chaos_patron(void);
extern void gain_level_reward(int chosen_reward);
extern bool set_shadow(int v);
extern bool set_tim_esp(int v);
extern bool tgp_pt(int *x, int * y);
extern bool tgt_pt (int *x, int *y);
extern bool gain_random_corruption(int choose_mut);
extern void dump_corruptions(FILE *OutFile);
extern void do_poly_self(void);
extern void do_poly_wounds(void);
extern bool curse_weapon(void);
extern bool curse_armor(void);
extern void random_resistance(object_type * q_ptr, bool is_scroll, int specific);
extern bool lose_corruption(int choose_mut);
extern void great_side_effect(void);
extern void nasty_side_effect(void);
extern void deadly_side_effect(bool god);
extern void godly_wrath_blast(void);
extern int interpret_grace(void);
extern int interpret_favor(void);
extern void make_wish(void);
extern bool set_sliding(s16b v);

/* levels.c */
extern bool get_level_desc(char *buf);
extern void get_level_flags();
extern bool get_dungeon_name(char *buf);
extern bool get_dungeon_special(char *buf);
extern bool get_command(const char *file, char comm, char *param);
extern int get_branch();
extern int get_fbranch();
extern int get_flevel();
extern bool get_dungeon_save(char *buf);

/* ghost.c */
extern s16b place_ghost(void);
extern void make_bones(void);


/* wizard2.c */
extern void do_cmd_wiz_cure_all(void);
extern void do_cmd_wiz_named_friendly(int r_idx, bool slp);

/* notes.c */
extern cptr notes_file(void);
extern void output_note(char *final_note);
extern void add_note(char *note, char code);
extern void add_note_type(int note_number);

/* squeltch.c */
extern void squeltch_inventory(void);
extern void squeltch_grid(void);
extern void do_cmd_squeltch_options(void);



/*
 * Hack -- conditional (or "bizarre") externs
 */

#ifdef SET_UID
/* util.c */
extern void user_name(char *buf, int id);
#endif

#ifndef HAS_MEMSET
/* util.c */
extern char *memset(char*, int, huge);
#endif

#ifndef HAS_STRICMP
/* util.c */
extern int stricmp(cptr a, cptr b);
#endif

#ifndef HAS_USLEEP
/* util.c */
extern int usleep(huge usecs);
#endif

#ifdef MACINTOSH
/* main-mac.c */
/* extern void main(void); */
#endif

#ifdef WINDOWS
/* main-win.c */
/* extern int FAR PASCAL WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, ...); */
#endif


#ifdef ALLOW_REPEAT /* TNB */

/* util.c */
extern void repeat_push(int what);
extern bool repeat_pull(int *what);
extern void repeat_check(void);
extern void get_count(int number, int max);

#endif /* ALLOW_REPEAT -- TNB */

#ifdef ALLOW_EASY_OPEN /* TNB */

/* variable.c */
extern bool easy_open;
extern bool easy_tunnel;

/* cmd2.c */
extern bool easy_open_door(int y, int x);

#endif /* ALLOW_EASY_OPEN -- TNB */

#ifdef ALLOW_EASY_DISARM /* TNB */

/* variable.c */
extern bool easy_disarm;

/* cmd2.c */
extern bool do_cmd_disarm_aux(int y, int x, int dir);

#endif /* ALLOW_EASY_DISARM -- TNB */

extern bool easy_floor;

