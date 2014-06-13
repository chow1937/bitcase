#ifndef _CMD_H
#define _CMD_H

/*Status codes*/
#define CMD_OK 0
#define CMD_ERROR -1

/*Commands table size*/
#define CMD_TABLE_SIZE 20

typedef struct cmd cmd;
typedef int cmd_proc(cmd *c);
struct cmd {
    /*Command process function*/
    cmd_proc *proc;
    /*DB that command execute on*/
    db *d;
    /*Result of the command execute*/
    void *result;
    /*Command arg num and values*/
    int argc;
    char **argv;
};

/*APIs*/
int cmd_init_commands(hash_table *commands);
int cmd_set_procfun(hash_table *commands, cmd *c, char *cmd_name);
cmd *cmd_parser(server_t *server, char *cmd_str);
int cmd_execute(cmd *c);
int cmd_free(cmd *c);

/*Command process functions*/
int cmd_get(cmd *c);
int cmd_set(cmd *c);
int cmd_delete(cmd *c);
int cmd_update(cmd *c);

#endif
