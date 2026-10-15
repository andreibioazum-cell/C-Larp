/* Read-only view of the ban list for the admin Players tab: the panel has to
 * show who is already banned so a tap can lift the ban, and it has to list the
 * bans that survived on this device even when the cloud is unreachable.
 * ban_sync() talks HTTP, so the refresh runs on a worker thread - the UI only
 * ever reads the cache. */

#define BAN_LIST_RET_SLOTS 8

static char s_ban_ret[BAN_LIST_RET_SLOTS][LOGIN_NICK_MAX + 1];
static int s_ban_ret_pos = 0;
static volatile sig_atomic_t s_ban_local_loaded = 0;
static volatile sig_atomic_t s_ban_syncing = 0;
