#ifndef _CMD_H
#define _CMD_H

/*Status codes*/
#define CMD_OK 0
#define CMD_ERROR -1

typedef int cmd_proc(struct cmd *c);
typedef struct cmd {
    cmd_proc *proc;
    /*DB that command execute on*/
    db *d;
    /*Result of the command execute*/
    void *result;
    /*Command arg num and values*/
    int argc;
    char **argv;
} cmd;

/*APIs*/
cmd *cmd_parser(char *cmd_str);
int cmd_execute(cmd *c);
int cmd_free(cmd *c);

/*Command process functions*/
int cmd_get_proc(cmd *c);
int cmd_set_proc(cmd *c);
int cmd_delete_proc(cmd *c);
int cmd_update_proc(cmd *c);

#endif
