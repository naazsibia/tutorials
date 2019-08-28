/*
Makes a Binary Search tree to keep track of processes the tracer gets attached to. Deletes the process when it exits. 
When no processes are left, the tracer exits as well. Shows how to extract PID of cloned process by using: 
ptrace(PTRACE_GETEVENTMSG, child, NULL, (long) &newpid); // LOOK UP HOW TO SEE CURRENT PROCESS'S PID
*/



// dontfork.c, a little ptrace utility that traces all child process
// and exits only when the latest spawned child is dead
#include <assert.h>
#include <stdio.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <linux/ptrace.h>
#include <sys/prctl.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>

#define WSTOPEVENT(s) (s >> 16)
#define OPTS PTRACE_O_TRACEFORK | PTRACE_O_TRACEVFORK | PTRACE_O_TRACECLONE
#define ESRCH -1

typedef struct BST {
   int data;
   struct BST *lchild, *rchild;
} node;

void insert(node *, node *);
void inorder(node *);
void preorder(node *);
void postorder(node *);
node *delete(node *, int data);
node *findmin(node *);
node *findmax(node *);
node *search(node *, int, node **);
node *get_node();
int do_child(int argc, char **argv);
void sig_handler(int signo);
void setup_sighandlers();
int do_trace(pid_t child);

node* pids = NULL;
int childs = 1;

int main(int argc, char *argv[]) {
	pid_t child = fork();
	if(child == 0) {
		return do_child(argc - 1, argv + 1);
	} else {
		pids = get_node();
		pids->data = child;
		setup_sighandlers();
		return do_trace(child);
	}

	return 0;
}

/*
 Get new Node
 */
node *get_node() {
   node *temp;
   temp = (node *) malloc(sizeof(node));
   temp->lchild = NULL;
   temp->rchild = NULL;
   return temp;
}
/*
 This function is for creating a binary search tree
 */
void insert(node *root, node *new_node) {
   if (new_node->data < root->data) {
      if (root->lchild == NULL)
         root->lchild = new_node;
      else
         insert(root->lchild, new_node);
   }

   if (new_node->data > root->data) {
      if (root->rchild == NULL)
         root->rchild = new_node;
      else
         insert(root->rchild, new_node);
   }
}
node *findmin(node *n) {
	if (n == NULL) return NULL;
	if (n->lchild)
		return findmin(n->lchild);
	else
		return n;
}
node *findmax(node *n) {
	if (n == NULL) return NULL;
	if (n->rchild)
		return findmax(n->rchild);
	else
		return n;
}
node* delete(node *n, int data) {
	node* temp;
	if (n == NULL) return NULL; // Element not found

	if (data < n->data) {
		n->lchild = delete(n->lchild, data);
	} else if (data > n->data) {
		n->rchild = delete(n->rchild, data);
	} else {
		/* Now We can delete this node and replace with either minimum element
	 in the right sub tree or maximum element in the left subtree */
		if (n->rchild && n->lchild) {
			temp = findmin(n->rchild);
			n->data = temp->data;
			n->rchild = delete(n->rchild, temp->data);
		} else {
			/* If there is only one or zero children then we can directly
	 		remove it from the tree and connect its parent to its child */
			temp = n;
			if (n->lchild == NULL)
				n = n->rchild;
			else if (n->rchild == NULL)
				n = n->lchild;
			free(temp);
		}
	}
	return n;
}
/*
 This function is for searching the node from
 binary Search Tree
 */
node *search(node *root, int key, node **parent) {
   node *temp;
   temp = root;
   while (temp != NULL) {
      if (temp->data == key) {
         fprintf(stderr, "\nThe %d Element is Present\n", temp->data);
         return temp;
      }
      *parent = temp;

      if (temp->data > key)
         temp = temp->lchild;
      else
         temp = temp->rchild;
   }
   return NULL;
}
/*
 This function displays the tree in inorder fashion
 */
void inorder(node *temp) {
   if (temp != NULL) {
      inorder(temp->lchild);
      fprintf(stderr, "%d\n", temp->data);
      inorder(temp->rchild);
   }
}
/*
 This function displays the tree in preorder fashion
 */
void preorder(node *temp) {
   if (temp != NULL) {
      fprintf(stderr, "%d\n", temp->data);
      preorder(temp->lchild);
      preorder(temp->rchild);
   }
}

/*
 This function displays the tree in postorder fashion
 */
void postorder(node *temp) {
   if (temp != NULL) {
      postorder(temp->lchild);
      postorder(temp->rchild);
      fprintf(stderr, "%d\n", temp->data);
   }
}

void setup_sighandlers() {
	if (signal(SIGABRT, sig_handler) == SIG_ERR)
		fprintf(stderr, "can't catch SIGABRT\n");
	if (signal(SIGALRM, sig_handler) == SIG_ERR)
		fprintf(stderr, "can't catch SIGALRM\n");
	if (signal(SIGBUS, sig_handler) == SIG_ERR)
		fprintf(stderr, "can't catch SIGBUS\n");
	if (signal(SIGCONT, sig_handler) == SIG_ERR)
		fprintf(stderr, "can't catch SIGCONT\n");
	if (signal(SIGFPE, sig_handler) == SIG_ERR)
		fprintf(stderr, "can't catch SIGFPE\n");
	if (signal(SIGHUP, sig_handler) == SIG_ERR)
		fprintf(stderr, "can't catch SIGHUP\n");
	if (signal(SIGILL, sig_handler) == SIG_ERR)
		fprintf(stderr, "can't catch SIGILL\n");
	if (signal(SIGINT, sig_handler) == SIG_ERR)
		fprintf(stderr, "can't catch SIGINT\n");
	if (signal(SIGPIPE, sig_handler) == SIG_ERR)
		fprintf(stderr, "can't catch SIGPIPE\n");
	if (signal(SIGQUIT, sig_handler) == SIG_ERR)
		fprintf(stderr, "can't catch SIGQUIT\n");
	if (signal(SIGSEGV, sig_handler) == SIG_ERR)
		fprintf(stderr, "can't catch SIGSEGV\n");
	if (signal(SIGTERM, sig_handler) == SIG_ERR)
		fprintf(stderr, "can't catch SIGTERM\n");
	if (signal(SIGTSTP, sig_handler) == SIG_ERR)
		fprintf(stderr, "can't catch SIGTSTP\n");
	if (signal(SIGTTIN, sig_handler) == SIG_ERR)
		fprintf(stderr, "can't catch SIGTTIN\n");
	if (signal(SIGTTOU, sig_handler) == SIG_ERR)
		fprintf(stderr, "can't catch SIGTTOU\n");
	if (signal(SIGUSR1, sig_handler) == SIG_ERR)
		fprintf(stderr, "can't catch SIGUSR1\n");
	if (signal(SIGUSR2, sig_handler) == SIG_ERR)
		fprintf(stderr, "can't catch SIGUSR2\n");
	if (signal(SIGUSR2, sig_handler) == SIG_ERR)
		fprintf(stderr, "can't catch SIGUSR2\n");
	if (signal(SIGPOLL, sig_handler) == SIG_ERR)
		fprintf(stderr, "can't catch SIGPOLL\n");
	if (signal(SIGPROF, sig_handler) == SIG_ERR)
		fprintf(stderr, "can't catch SIGPROF\n");
	if (signal(SIGSYS, sig_handler) == SIG_ERR)
		fprintf(stderr, "can't catch SIGSYS\n");
	if (signal(SIGTRAP, sig_handler) == SIG_ERR)
		fprintf(stderr, "can't catch SIGTRAP\n");
	if (signal(SIGURG, sig_handler) == SIG_ERR)
		fprintf(stderr, "can't catch SIGURG\n");
	if (signal(SIGVTALRM, sig_handler) == SIG_ERR)
		fprintf(stderr, "can't catch SIGVTALRM\n");
	if (signal(SIGXCPU, sig_handler) == SIG_ERR)
		fprintf(stderr, "can't catch SIGXCPU\n");
	if (signal(SIGXFSZ, sig_handler) == SIG_ERR)
		fprintf(stderr, "can't catch SIGXFSZ\n");
}

void sig_postorder(node *temp, int signo) {
	if (temp != NULL) {
		sig_postorder(temp->lchild, signo);
		sig_postorder(temp->rchild, signo);
		fprintf(stderr, "Delivering %d to %d\n", signo, temp->data);

		int status = 0;
		kill(temp->data, SIGSTOP);
		if (0 != ptrace(PTRACE_DETACH, temp->data, NULL, NULL))
			goto cleanup;
		kill(temp->data, SIGCONT);
		kill(temp->data, signo);
		kill(temp->data, SIGSTOP);

		if (ESRCH == ptrace(PTRACE_ATTACH, temp->data, NULL, NULL))
			goto cleanup;

		assert(0 == ptrace(PTRACE_SETOPTIONS, temp->data, NULL, OPTS));
		ptrace(PTRACE_SYSCALL, temp->data, NULL, NULL);

		return;

		cleanup:
			fprintf(stderr, "Child %d exited\n", temp->data);
			pids = delete(pids, temp->data);
			childs--;
			if (!childs) exit(0);
	}
}

void sig_handler(int signo) {
	sig_postorder(pids, signo);
}

int do_trace(pid_t child) {
	long newpid;
	int status;

	waitpid(child, &status, 0);
	assert(WIFSTOPPED(status));
	assert(0 == ptrace(PTRACE_SETOPTIONS, child, NULL, OPTS));
	ptrace(PTRACE_SYSCALL, child, NULL, NULL);

	while(childs) {
		child = waitpid(-1, &status, __WALL);

		if (WSTOPEVENT(status) == PTRACE_EVENT_FORK ||
			WSTOPEVENT(status) == PTRACE_EVENT_VFORK ||
			WSTOPEVENT(status) == PTRACE_EVENT_CLONE) {
			printf("My process id %d\n", getpid());
			ptrace(PTRACE_GETEVENTMSG, child, NULL, (long) &newpid);
			ptrace(PTRACE_SYSCALL, newpid, NULL, NULL);

			fprintf(stderr, "Attached to offspring %ld\n", newpid);
			node* n = get_node();
			n->data = newpid;
			insert(pids, n);
			childs++;
		} else {
			if(WIFEXITED(status)) {
				fprintf(stderr, "Child %d exited\n", child);
				pids = delete(pids, child);
				childs--;
			}
		}

		ptrace(PTRACE_SYSCALL, child, NULL, NULL);
	}
	return 0;
}

int do_child(int argc, char **argv) {
	char *args [argc + 1];
	int i;
	for (i = 0; i < argc; i++)
		args[i] = argv[i];
	args[argc] = NULL;

	assert(0 == ptrace(PTRACE_TRACEME));
	kill(getpid(), SIGSTOP);
	return execvp(args[0], args);
}