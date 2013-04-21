/* File: variable.c */

/*
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies.  Other copyrights may also apply.
 */

#include "angband.h"


/*
 * Hack -- Link a copyright message into the executable
 */
cptr copyright[5] = {
	"Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Keoneke",
	"",
	"This software may be copied and distributed for educational, research,",
	"and not for profit purposes provided that this copyright and statement",
	"are included in all such copies.  Other copyrights may also apply."
};


/*
 * Executable version
 */
byte version_major = KAM_VERSION_MAJOR;
byte version_minor = KAM_VERSION_MINOR;
byte version_patch = KAM_VERSION_PATCH;
byte version_extra = VERSION_EXTRA;

/*
 * Savefile version
 */
byte sf_major; /* Savefile's "version_major" */
byte sf_minor; /* Savefile's "version_minor" */
byte sf_patch; /* Savefile's "version_patch" */
byte sf_extra; /* Savefile's "version_extra" */

/*
 * Savefile information
 */
u32b sf_xtra; /* Operating system info */
u32b sf_when; /* Time when savefile created */
u16b sf_lives; /* Number of past "lives" with this file */
u16b sf_saves; /* Number of "saves" during this life */

/*
 * Run-time arguments
 */
bool arg_fiddle; /* Command arg -- Request fiddle mode */
bool arg_wizard; /* Command arg -- Request wizard mode */
bool arg_sound;	/* Command arg -- Request special sounds */
bool arg_graphics; /* Command arg -- Request graphics mode */
bool arg_force_original; /* Command arg -- Request original keyset */
bool arg_force_roguelike; /* Command arg -- Request roguelike keyset */

/*
 * Various things
 */

bool character_generated; /* The character exists */
bool character_dungeon;	/* The character has a dungeon */
bool character_loaded; /* The character was loaded from a savefile */
bool character_saved; /* The character was just saved to a savefile */

s16b character_icky; /* The game is in an icky full screen mode */
bool character_xtra; /* The game is in an icky startup mode */

u32b seed_flavor; /* Hack -- consistent object colors */
u32b seed_town;	/* Hack -- consistent town layout */
u32b seed_dungeon; /* Simulate persistent dungeons */
u32b seed_wild;	/* Persistent wilderness */

s16b num_repro;	/* Current reproducer count */
s16b object_level; /* Current object creation level */
s16b monster_level;	/* Current monster creation level */

s16b arena_monsters[MAX_ARENAS][MAX_ARENA_MONS]; /* -KMW- */

s32b turn; /* Current game turn */

s32b old_turn; /* Hack -- Level feeling counter */

s32b old_resting_turn; /* Hack -- Resting turn counter */

bool use_sound;	/* The "sound" mode is enabled */
bool use_graphics; /* The "graphics" mode is enabled */

s16b signal_count; /* Hack -- Count interupts */

bool msg_flag; /* Player has pending message */

bool inkey_base; /* See the "inkey()" function */
bool inkey_xtra; /* See the "inkey()" function */
bool inkey_scan; /* See the "inkey()" function */
bool inkey_flag; /* See the "inkey()" function */

s16b coin_type;	/* Hack -- force coin type */

bool opening_chest;	/* Hack -- prevent chest generation */

bool shimmer_monsters; /* Hack -- optimize multi-hued monsters */
bool shimmer_objects; /* Hack -- optimize multi-hued objects */

bool repair_mflag_born;	/* Hack -- repair monster flags (born) */
bool repair_mflag_nice;	/* Hack -- repair monster flags (nice) */
bool repair_mflag_show;	/* Hack -- repair monster flags (show) */
bool repair_mflag_mark;	/* Hack -- repair monster flags (mark) */

s16b o_max = 1;	/* Number of allocated objects */
s16b o_cnt = 0;	/* Number of live objects */

s16b m_max = 1;	/* Number of allocated monsters */
s16b m_cnt = 0;	/* Number of live monsters */

s16b m_pet_num = 0;	/* Number of pets that are active at this time */

s16b m_generators = 0; /* Number of monster generators */

/* 
 * Screen size (in characters)
 * This should be corrected (if non-default) by main-xxx.c
 */
s16b screen_x = 80;
s16b screen_y = 24;

/*
 * Height of dungeon map on screen.
 */
s16b SCREEN_HGT = 22;
s16b SCREEN_WID = 66;

/*
 * Dungeon variables
 */

s16b feeling; /* Most recent feeling */
s16b rating; /* Level's current rating */
s16b pet_rating; /* Amount of sacred pets on the level. */

bool good_item_flag; /* True if "Artifact" on this level */

bool closing_flag; /* Dungeon is closing */


/*
 * Player info
 */
int player_uid;
int player_euid;
int player_egid;


/*
 * Buffer to hold the current savefile name
 */
char savefile[1024];


/*
 * Array of grids lit by player lite (see "cave.c")
 */
s16b lite_n;
byte lite_y[LITE_MAX];
byte lite_x[LITE_MAX];

/*
 * Array of grids viewable to the player (see "cave.c")
 */
s16b view_n;
byte view_y[VIEW_MAX];
byte view_x[VIEW_MAX];

/*
 * Array of grids for use by various functions (see "cave.c")
 */
s16b temp_n;
byte temp_y[TEMP_MAX];
byte temp_x[TEMP_MAX];


/*
 * Number of active macros.
 */
s16b macro__num;

/*
 * Array of macro patterns [MACRO_MAX]
 */
cptr *macro__pat;

/*
 * Array of macro actions [MACRO_MAX]
 */
cptr *macro__act;

/*
 * Array of macro types [MACRO_MAX]
 */
bool *macro__cmd;

/*
 * The number of quarks
 */
s16b quark__num;

/*
 * The pointers to the quarks [QUARK_MAX]
 */
cptr *quark__str;


/*
 * The next "free" index to use
 */
u16b message__next;

/*
 * The index of the oldest message (none yet)
 */
u16b message__last;

/*
 * The next "free" offset
 */
u16b message__head;

/*
 * The offset to the oldest used char (none yet)
 */
u16b message__tail;

/*
 * The array of offsets, by index [MESSAGE_MAX]
 */
u16b *message__ptr;

/*
 * An array of message priorities, corresponds to ``message__ptr''
 */
byte *message__pty;

/*
 * The array of chars, by offset [MESSAGE_BUF]
 */
char *message__buf;


/*
 * The array of window pointers
 */
term *angband_term[8];


/*
 * Standard window names
 */
char angband_term_name[8][16] = {
	"Angband",
	"Term-1",
	"Term-2",
	"Term-3",
	"Term-4",
	"Term-5",
	"Term-6",
	"Term-7"
};


/*
 * Global table of color definitions (mostly zeros)
 */
byte angband_color_table[256][4] = {
	{0x00, 0x00, 0x00, 0x00}, /* TERM_DARK */
	{0x00, 0xFF, 0xFF, 0xFF}, /* TERM_WHITE */
	{0x00, 0x80, 0x80, 0x80}, /* TERM_SLATE */
	{0x00, 0xFF, 0x80, 0x00}, /* TERM_ORANGE */
	{0x00, 0xC0, 0x00, 0x00}, /* TERM_RED */
	{0x00, 0x00, 0x80, 0x40}, /* TERM_GREEN */
	{0x00, 0x00, 0x40, 0xFF}, /* TERM_BLUE */
	{0x00, 0x80, 0x40, 0x00}, /* TERM_UMBER */
	{0x00, 0x60, 0x60, 0x60}, /* TERM_L_DARK */
	{0x00, 0xC0, 0xC0, 0xC0}, /* TERM_L_WHITE */
	{0x00, 0xFF, 0x00, 0xFF}, /* TERM_VIOLET */
	{0x00, 0xFF, 0xFF, 0x00}, /* TERM_YELLOW */
	{0x00, 0xFF, 0x00, 0x00}, /* TERM_L_RED */
	{0x00, 0x00, 0xFF, 0x00}, /* TERM_L_GREEN */
	{0x00, 0x00, 0xFF, 0xFF}, /* TERM_L_BLUE */
	{0x00, 0xC0, 0x80, 0x40} /* TERM_L_UMBER */
};


/*
 * Standard sound names
 */
char angband_sound_name[SOUND_MAX][16] = {
	"",
	"hit",
	"miss",
	"flee",
	"drop",
	"kill",
	"level",
	"death",
	"study",
	"teleport",
	"shoot",
	"quaff",
	"zap",
	"walk",
	"tpother",
	"hitwall",
	"eat",
	"store1",
	"store2",
	"store3",
	"store4",
	"dig",
	"opendoor",
	"shutdoor",
	"tplevel"
};


#ifdef MONSTER_FLOW

/*
 * The array of cave grid flow "cost" values
 */
byte cave_cost[DUNGEON_HGT][DUNGEON_WID];

/*
 * The array of cave grid flow "when" stamps
 */
byte cave_when[DUNGEON_HGT][DUNGEON_WID];

#endif /* MONSTER_FLOW */

/*
 * The array of cave grid info flags
 */
byte cave_info[DUNGEON_HGT][DUNGEON_WID];

/*
 * The array of cave grid feature codes
 */
byte cave_feat[DUNGEON_HGT][DUNGEON_WID];


/*
 * The array of cave grid object indexes
 *
 * This array gives a pointer to the object in the o_list.
 */
object_type *cave_o_idx[DUNGEON_HGT][DUNGEON_WID];

/*
 * The array of cave grid monster indexes
 *
 * Note that this array yields the index of the monster or player in a grid,
 * where negative numbers are used to represent the player, positive numbers
 * are used to represent a monster, and zero is used to indicate "nobody".
 * This array replicates the information contained in the monster list and
 * the player structure, but provides extremely fast determination of which,
 * if any, monster or player is in any given grid.
 */
s16b cave_m_idx[DUNGEON_HGT][DUNGEON_WID];


/*
 * The linked list of dungeon objects
 */
object_type *o_list = NULL;

/*
 * The array of dungeon monsters
 */
monster_type m_list[MAX_M_IDX];

/*
 * The array of monster generators
 */

generator gen_list[MAX_GENERATORS];

/* 
 * Remembered recipes. 
 */
byte recipe_recall[MAX_RECIPES];

/*
 * Quest status, quest-to-v_ptr array.
 */
byte quest_status[MAX_QUESTS];
vault_type *q_v_ptrs[MAX_QUESTS];
byte max_quests;

/*
 * Gift status. This is currently used only when giving a gift to a player.
 */
byte rewards[MAX_REWARDS];

/*
 * Current bounties. An array of tuples of two, with the first being the
 * r_idx of the monster, and the second the monster's worth.
 */
s16b bounties[MAX_BOUNTIES][2];

/*
 * Player's spell list.
 */

spell spells[MAX_SPELLS];
u16b spell_num = 0;

/*
 * List of possible intrinsic powers.
 */

spell powers[MAX_POWERS];
u16b power_num = 0;

/* 
 * List of item activations.
 */

spell activations[MAX_ACTIVATIONS];
u16b activation_num;

/*
 * Random artifacts.
 */

random_artifact random_artifacts[MAX_RANDARTS];

/*
 * The stores [MAX_STORES]
 */
store_type *store;

/*
 * The player's inventory [INVEN_TOTAL]
 */
object_type *inventory = NULL;

/*
 * Helpfull pointers to currently used items.
 */
object_type *equipment[EQUIP_MAX];


/*
 * The size of "alloc_kind_table" (at most MAX_K_IDX * 4)
 */
s16b alloc_kind_size;

/*
 * The entries in the "kind allocator table"
 */
alloc_entry *alloc_kind_table;


/*
 * The size of "alloc_race_table" (at most MAX_R_IDX)
 */
s16b alloc_race_size;

/*
 * The entries in the "race allocator table"
 */
alloc_entry *alloc_race_table;


/*
 * Specify attr/char pairs for visual special effects
 * Be sure to use "index & 0x7F" to avoid illegal access
 */
byte misc_to_attr[128];
char misc_to_char[128];


/*
 * Specify color for inventory item text display (by tval)
 * Be sure to use "index & 0x7F" to avoid illegal access
 */
byte tval_to_attr[128];


/*
 * Current (or recent) macro action
 */
char macro_buffer[1024];


/*
 * Keymaps for each "mode" associated with each keypress.
 */
cptr keymap_act[KEYMAP_MODES][256];



/*** Player information ***/

/*
 * Pointer to the player tables
 * (sex, race, class, magic)
 */
player_sex *sp_ptr;
player_race *rp_ptr;
player_class *cp_ptr;

/*
 * The player other record (static)
 */
static player_other player_other_body;

/*
 * Pointer to the player other record
 */
player_other *op_ptr = &player_other_body;

/*
 * The player info record (static)
 */
static player_type player_type_body;

/*
 * Pointer to the player info record
 */
player_type *p_ptr = &player_type_body;


/*
 * The vault generation arrays
 */
header *v_head;
vault_type *v_info;
char *v_name;
char *v_text;
char *q_text;
char *vm_text;

/*
 * The terrain feature arrays
 */
header *f_head;
feature_type *f_info;
char *f_name;
char *f_text;

/*
 * The object kind arrays
 */
header *k_head;
object_kind *k_info;
char *k_name;
char *k_text;

/*
 * The artifact arrays
 */
header *a_head;
artifact_type *a_info;
char *a_name;
char *a_text;

/*
 * The ego-item arrays
 */
header *e_head;
ego_item_type *e_info;
char *e_name;
char *e_text;


/*
 * The monster race arrays
 */
header *r_head;
monster_race *r_info;
char *r_name;
char *r_text;
char *sayings_text;


/*
 * Hack -- The special Angband "System Suffix"
 * This variable is used to choose an appropriate "pref-xxx" file
 */
cptr ANGBAND_SYS = "xxx";

/*
 * Hack -- The special Angband "Graphics Suffix"
 * This variable is used to choose an appropriate "graf-xxx" file
 */
cptr ANGBAND_GRAF = "old";

/*
 * Path name: The main "lib" directory
 * This variable is not actually used anywhere in the code
 */
cptr ANGBAND_DIR;

/*
 * High score files (binary)
 * These files may be portable between platforms
 */
cptr ANGBAND_DIR_APEX;

/*
 * Bone files for player ghosts (ascii)
 * These files are portable between platforms
 */
cptr ANGBAND_DIR_BONE;

/*
 * Binary image files for the "*_info" arrays (binary)
 * These files are not portable between platforms
 */
cptr ANGBAND_DIR_DATA;

/*
 * Textual template files for the "*_info" arrays (ascii)
 * These files are portable between platforms
 */
cptr ANGBAND_DIR_EDIT;

/*
 * Various extra files (ascii)
 * These files may be portable between platforms
 */
cptr ANGBAND_DIR_FILE;

/*
 * Help files (normal) for the online help (ascii)
 * These files are portable between platforms
 */
cptr ANGBAND_DIR_HELP;

/*
 * Help files (spoilers) for the online help (ascii)
 * These files are portable between platforms
 */
cptr ANGBAND_DIR_INFO;

/*
 * Savefiles for current characters (binary)
 * These files are portable between platforms
 */
cptr ANGBAND_DIR_SAVE;

/*
 * User "preference" files (ascii)
 * These files are rarely portable between platforms
 */
cptr ANGBAND_DIR_USER;

/*
 * Various extra files (binary)
 * These files are rarely portable between platforms
 */
cptr ANGBAND_DIR_XTRA;


/*
 * Total Hack -- allow all items to be listed (even empty ones)
 * This is only used by "do_cmd_inven_e()" and is cleared there.
 */
bool item_tester_full;


/*
 * Here is a "pseudo-hook" used during calls to "get_item()" and
 * "show_inven()" and "show_equip()", and the choice window routines.
 */
byte item_tester_tval;


/*
 * Here is a "hook" used during calls to "get_item()" and
 * "show_inven()" and "show_equip()", and the choice window routines.
 */
bool(*item_tester_hook) (object_type *);



/*
 * Current "comp" function for ang_sort()
 */
bool(*ang_sort_comp) (vptr u, vptr v, int a, int b);


/*
 * Current "swap" function for ang_sort()
 */
void (*ang_sort_swap) (vptr u, vptr v, int a, int b);



/*
 * Hack -- function hook to restrict "get_mon_num_prep()" function
 */
bool(*get_mon_num_hook) (int r_idx);



/*
 * Hack -- function hook to restrict "get_obj_num_prep()" function
 */
bool(*get_obj_num_hook) (int k_idx);


/* XXX Mega-Hack - See main-win.c */
bool angband_keymap_flag = TRUE;


/*
 * Hack -- don't punish thefts. 
 */
bool hack_punish_theft = TRUE;


/*
 * Hack -- Extra flags to control object_desc()
 */
byte object_desc_mode = 0;


/*
 * Hack -- object_unabsortb() should combine store items
 */
bool store_combine_flag = TRUE;

