/* LinkList.H -- ANSI C Linked List Container Type
                 Handles Lists, Queues, Stacks, Splay Trees, 
		   and custom list types via one consistant interface.

	       - Written by Jeff Hay, jrhay@lanl.gov
	         - If you find errors in this library, please let me know!

   What It Does:
         - Creates, maintains, and destroys any type of linked memory structure
           (singly/doubly linked lists, queues, stacks, etc) effeciently and
           without memory leaks.
	 - Any user-specified data may be held within structure nodes.
	 - Allows user-definable memory allocation and deallocation routines
           to be used in place of standard malloc() and free() calls.
	 - Any number and types of linked memory structures may be created
           (subject only to memory limitations)
	 - "Should be" portable to any ANSI C compilent platform

   Stuff To Do Yet:
         - Add functionallity to be able to interrupt list maintence routines
           at any point and still maintain list continuity (for multi-processor
           shared memory environments)

  The object file compiled from this library is around 5K depending on 
  operating platform.

*/
#ifndef __c_LINKLIST__
#define __c_LINKLIST__

/* 
   NOTE:  All functions assume the list data structures are UNTOUCHED expect
          by functions in this library.  Bad things could happen if you muck 
	  with them at all.  (unless you know what you are doing.... ;)

  This library has been through several incarnations, each one adding features
  while maintaining compatibility with the previous versions.  The result is a 
  confusing number of functions.  For new programs, the only functions you need
  are:
          List Creation     - NewListAlloc(), AddNode()
	  List Destruction  - FreeList(), DelNode()
	  List Transversal  - NextNode(), PrevNode(), IndexNode()
	  List Manipulation - GetNode(), GetNodeData()

  All other functions are either internal-use functions or obsolete definitions
  to maintain source compatiblity for programs written to older versions of 
  this  library.

  Note also that if using the library to maintain binary search trees, the
  List Transversal functions should not normally be used.
   
  See the last section of this header file for extended documentation on all 
  functions.
*/

/* Will use the DALLOC library if "dalloc.h" is included on the compiler
   command line */

/* Uncomment the following line to compile an executable from LinkList.C that
   will demonstrate and test the functions contained in this library. (or 
   define LINKLIST_TEST on the compiler command line ("make linklist") */
/* #define LINKLIST_TEST 1  */

/* -------
   Include files used by this library
------- */
#include <stdio.h>
#include <stdlib.h>

/* --------
   List Flags and Parameters
-------- */

/* List Parameters */
#define LISTADDCURR   0x300  /* Add New Node At Current Record In List */
#define LISTADDHEAD   0x100  /* Add New Nodes At Head Of List */
#define LISTADDTAIL   0x200  /* Add New Nodes At Tail Of List */
#define LISTADDSPLAY  0x400  /* Add New Nodes As A Splay Tree */
#define LISTDELCURR   0x030  /* Delete Nodes At Current Record */
#define LISTDELHEAD   0x010  /* Delete Nodes At Head Of List */
#define LISTDELTAIL   0x020  /* Delete Nodes At Tail Of List */
#define LISTDELSPLAY  0x040  /* Delete Nodes As A Splay Tree */
#define LISTREADCURR  0x003  /* Read List At Current Node */
#define LISTREADHEAD  0x001  /* Read Head Of List */
#define LISTREADTAIL  0x002  /* Read Tail Of List */

#define LISTDELREAD   0x1000 /* Delete Node On Reading */
#define LISTCIRCULAR  0x2000 /* Circular List - Head->Next=Tail, etc */
#define LISTBTREE     0x4000 /* List is actually a binary tree */

/* Masks of List Parameters */
#define LISTADDMASK  0xF00  /* Add New Node Method */
#define LISTDELMASK  0x0F0  /* Delete Node Method */
#define LISTREADMASK 0x00F  /* Read Node Method */
#define LISTFLAGMASK 0xF000 /* Operation Flags */

/* Common Data Structure Types */
#define LIST (LISTADDCURR | LISTREADCURR | LISTDELCURR) 
#define FIFO (LISTADDTAIL | LISTREADHEAD | LISTDELHEAD)
#define LIFO (LISTADDHEAD | LISTREADHEAD | LISTDELHEAD)
#define QUEUE (FIFO | LISTDELREAD)
#define STACK (LIFO | LISTDELREAD)
#define CIRCULAR_QUEUE (QUEUE | LISTCIRCULAR)
#define STREE (LISTBTREE | LISTADDSPLAY | LISTDELSPLAY | LISTREADCURR)

/* --------
   Possible return values of functions in this library 
-------- */
#define LLIST_NOERROR 0    /* No problem! */
#define LLIST_NULL    1    /* Bad value passed to function */
#define LLIST_ERROR  -1    /* Misc. program/library error.  Serious trouble! */

#define LLIST_OK LLIST_NOERROR   /* duplicate definitions for compatibility */
#define LLIST_BADVALUE LLIST_NULL

/* --------
   List data structures
-------- */

typedef void (*ListFreeFunc)(void *); 
/* Function to release memory stored within a list (free() syntax) */

typedef void *(* ListAlloc)(size_t size);
/* Memory allocation procedure to use for this list (malloc() syntax) */

typedef int (* NodeCompareFunc)(void *, void *);
/* Function used to compare nodes for list sorting.  The two passed pointers
   are two data elements from nodes of a list.  CompareFunc must return:
                1 iff First Node > Second Node
                0 iff First Node = Second Node
               -1 iff First Node < Second Node  
   Results are undefined if one or both pointers are NULL. (note that this
   definition is compatible with strcmp() and related functions) */ 

typedef int (* ListDumpFunc)(void *);
/* Function to dump the data of a node to the screen */

typedef struct ListNode* listnodePtr;
typedef struct ListNode
{  
  void        *Data;  /* Data stored at this node (user-defined) */
  listnodePtr Next,   /* Next Node in List, Right Child in Binary Tree */
              Prev;   /* Previous Node in List, Left Child in Binary Tree */
} listnode;

typedef struct LList* listPtr;
typedef struct LList
{  
  listnodePtr  Current,  /* Last Accessed Node */ 
               Head,     /* Head of List, Root of Binary Tree */ 
               Tail;     /* Tail of List */
  int          Size,     /* Number of nodes in List or Binary Tree */ 
               Flags;    /* Flags associated with List/Tree */
  ListAlloc    memalloc; /* malloc()-type procedure to use */
  ListFreeFunc memfree;  /* free()-type procedure to use */
  NodeCompareFunc compare; /* Function to use to compare nodes */
} llist;

/* -------
   Make sure we have DALLOC (or similarly-named macros)
-------- */

#ifndef DMALLOC
#define DMALLOC malloc
#endif
#ifndef DFREE
#define DFREE dfree
#endif
#ifndef DCOUNT
#define DCOUNT dcount
#endif

/* --------
  Function Prototypes
-------- */

listPtr NewListAlloc(int ListType, ListAlloc Lalloc, ListFreeFunc Lfree,
		     NodeCompareFunc Cfunc);
/* Create a brand new list structure.  The structrue is defined by
   ListType, which is a logical OR of the List Parameters defined above.
   Use the specified allocation and free function to allocate dynamic
   memory to the list.  If "alloc" or "free" are NULL, default to
   malloc() and free() (respectively) from stdlib.  If "Cfunc" is NULL, don't
   do comparisons.

   Returns
        Pointer to a new list
        NULL on error (Lalloc() procedure failed) */

#define NewList(Type) NewListAlloc(Type, NULL, NULL, NULL)
/* Macro definition of: listPtr NewList(int ListType); 
   for compatibility with previous versions of library */

listnodePtr NewListNode(listPtr List, void *Data);
/* Creates a new node for the specified list.  Memory is allocated with
   the list alloc procedure.  Data is a pointer to any kind of data or may
   be NULL.  If List is NULL, node is created using malloc().
   Returns 
        Pointer to the new node
        NULL on error (alloc failed) */

#define NewNode(Data) NewListNode(NULL, Data)
/* Macro definition of: listnodePtr NewNode(void *Data);
   for compatibility with previous versions of library */

void *GetNode(listPtr List);
/* Reads the next node in the list (as specified at list creation with one of 
   the LISTREAD* properties).  If list has unknown LISTREAD* property, returns
   as LISTREADCURR.  If LISTDELREAD is a property of the list, the
   node is automatically deleted (note that the node DATA is *not* free()'d).  

   Returns
        Pointer to the node data
        NULL if List is NULL or empty (or list read property is unknown) */

void *FindNode(listPtr List, void *Data);
/* Finds a node in the list for which the list compare function specified at
   list creation returns 0 when compared with "Data".  Sets List->Current to
   the found node, and returns found node's data.

   Returns
       Pointer to the node data
       NULL if list empty
            if no list compare function
            if no node matching data was found in list
*/

void *BTFind(listPtr List, void *Data);
/*  Performs "FindNode" operation when list is a binary tree; called 
    automatically by FindNode() when list has LISTBTREE property set.  */

void *GetNodeData(listnodePtr Node);
/* Returns the data contained in the specified node, or NULL if Node is empty*/

int AddNode(listPtr List, listnodePtr Node);
/* Adds a node to the list in the location specified at list creation by one of
   the LISTADD* properties.  If the LISTADD property is unknown for any reason
   (program error), the node is added at the current list position.  Node must
   be created by NewListNode.  Current list pointer is set to the newly added
   node.  
   Returns
        LLIST_NOERROR on success
	LLIST_NULL if either List or Node are NULL 
	LLIST_ERROR on undefined program error */

int InsertList(listPtr List, listnodePtr Node);
int HeadList(listPtr List, listnodePtr Node);
int TailList(listPtr List, listnodePtr Node);
int SplayInsertList(listPtr List, listnodePtr Node);
/* These functions add the specified node to the specified list at the either 
   the current position, the head of the list, the tail of the list, ,or in a
   splay pattern, respectively.  All assume the node has been init'd
   by NewNode() and all set List->Current to the newly added node.

   These functions should not normally be called directly by user programs
   (AddNode() will call the approrpiate function for the list)

   All return
        LLIST_NOERROR on success
	LLIST_NULL if either List or Node are NULL */

int DelNode(listPtr List);
/* Deletes a node from the list. The node deleted is specified at list creation
   by one of the LISTDEL* properties. If the LISTDEL property is unknown for
   any reson, the "current" node is deleted.  Current list position is set to 
   the next logical node in the list (or NULL).  Note: Note DATA is *not*
   deleted.

   Returns
        LLIST_NOERROR on success
	LLIST_NULL if List is NULL
	LLIST_ERROR on undefined program error */

int RemoveList(listPtr List);
int DelHeadList(listPtr List);
int DelTailList(listPtr List);
/* These functions delete the node at either the current list position,
   the head of the list, the tail of the list, or as a splay pattern,
   respectively.  All set List->Current to the next node in the list 
   (Node->Next).

   These functions should not normally be called directly by user programs
   (DelNode() will call the approrpiate function for the list)

   All Return
        LLIST_NOERROR on success
	LLIST_NULL if List is NULL */

void *NextNode(listPtr List);
/* Step to the next logical node in the list.  For lists with LISTBTREE set, 
   steps to the right child of the current node.

   Returns
        NULL if List or next node is empty
	Pointer to the next node's data on success */

void *PrevNode(listPtr List);
/* Step to the previous logical node in the list. For lists with LISTBTREE set,
   steps to the left child of the current node.

   Returns
        NULL if List or next node is empty
	Pointer to the next node's data on success */

void *IndexNode(listPtr List, int Index);
/* Step to the logical node numbed "Index" in the List.  The head of
   the list is index "1"; the tail is index "List->Size".  If LISTBTREE is set,
   this function always returns NULL (Indexing makes no sense).

   Returns
        NULL if List or indexed node is empty
	     if Index > size of list
	Pointer to the index node's data on success */

int FreeList(listPtr List, ListFreeFunc DataFree);
/* Deletes entire list structure and frees all memory.  List is undefined
   after this call.  "DataFree" is an optional function used to free the
   memory allocated for each node's data (ie, if the data is a dynamic 
   structure, etc); it may be NULL.  It will *not* be called for empty
   nodes (node->data == NULL).  If this function returns with anything 
   other then LLIST_NOERROR, an error was encountered deleting a node from
   the list.  List is in undefined state. */

void SwapList(listPtr List);
/* Swaps the current node in the list with the next node in the list.  No
   function if current node is tail of list.  */

void SortList(listPtr List);
/* Preforms a slow sort on the list.  Sort is handled in-place.  Current node
   is head of list after sort.  Does not  attempt to sort lists with LISTBTREE
   property set.
*/

void *SplayList(listPtr List, void *Data);
/* Performs a splay operation on the list.  The list is assumed to be a splay
   tree.  Finds the node in the tree that matches "Data" and moves that node
   (or the closest node less then data) to the root of the tree, preserving the
   inorder transversal of the tree.

   Returns
           Pointer to node data if found
           NULL if List is NULL
	        if "Data" not found in Tree */	       

int IntCompare(int *First, int* Second);
int StringCompare(char *First, char* Second);
int DoubleCompare(double *First, double* Second);
/* These are suitable NodeCompareFunc functions for three common types of
   nodes.  Provided just to make life a little easier... */

int DumpList(listPtr List, ListDumpFunc DataDump);
/* Print List data using the DataDump function for Node Data Elements */

#ifdef LINKLIST_TEST

int PrintList(listPtr List, char *DataFmt);
/*  Display the contents of a list.  Provided mainly as an example
   of how to transverse list if needed.  Written for an old version of
   this library and never updated. */

int PrintTree(listPtr List, char *DataFmt);
/*  Prints the contents and structure of a Tree using DataFmt as the printf() 
    format for Node Data Elements. Called by PrintList as appropriate. */

#endif

#endif  /* __c_LINKLIST__ */





