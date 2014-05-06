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
int cmd_init_commands(void);
int cmd_set_procfun(cmd *c, char *cmd_name);
cmd *cmd_parser(char *cmd_str);
int cmd_execute(cmd *c);
int cmd_free(cmd *c);

/*Command process functions*/
int cmd_get_proc(cmd *c);
int cmd_set_proc(cmd *c);
int cmd_delete_proc(cmd *c);
int cmd_update_proc(cmd *c);

#endif
