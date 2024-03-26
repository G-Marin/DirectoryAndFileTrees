
/*--------------------------------------------------------------------*/
/* checkerDT.c                                                        */
/* Author: Gabriel Marin & Kurt Lemai  */
/*--------------------------------------------------------------------*/

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "checkerDT.h"
#include "dynarray.h"
#include "path.h"



/* see checkerDT.h for specification */
boolean CheckerDT_Node_isValid(Node_T oNNode) {
   Node_T oNParent;
   Path_T oPNPath;
   Path_T oPPPath;


   /* Sample check: a NULL pointer is not a valid node */
   if(oNNode == NULL) {
      fprintf(stderr, "A node is a NULL pointer\n");
      return FALSE;
   }

   /* Sample check: parent's path must be the longest possible
      proper prefix of the node's path */
   oNParent = Node_getParent(oNNode);
   if(oNParent != NULL) {
      oPNPath = Node_getPath(oNNode);
      oPPPath = Node_getPath(oNParent);

      if(Path_getSharedPrefixDepth(oPNPath, oPPPath) !=
         Path_getDepth(oPNPath) - 1) {
         fprintf(stderr, "P-C nodes don't have P-C paths: (%s) (%s)\n",
                 Path_getPathname(oPPPath), Path_getPathname(oPNPath));
         return FALSE;
      }
   }

   return TRUE;
}


/* Compare the ulCount of the tree to the nodeCount. Returns FALSE if the ulCount does not matcht the number of nodes in the tree. */
static boolean CheckerDT_countCheck(size_t ulCount, size_t* nodeCount){

   assert(nodeCount != NULL);

   if(ulCount != *nodeCount){
      fprintf(stderr, "ulCount does not match number of nodes in tree.\n ulCount: %lu\n", (unsigned long int) ulCount);

      fprintf(stderr, "Number of Nodes: %lu \n", (unsigned long int) *nodeCount);

      *nodeCount = 0;
      return FALSE;
   }
   *nodeCount = 0;
   return TRUE;
}


/* Compares each child of oNNode and returns FALSE if any two nodes are the same. */
static boolean CheckerDT_childCheck(Node_T oNNode){

   size_t compareIndex;
   size_t childIndex;
   size_t children;
   Node_T oNChild = NULL;
   Node_T twoChild = NULL;
   Path_T onePath;
   Path_T twoPath;


   children = Node_getNumChildren(oNNode);
   

   for(childIndex = 0; childIndex < children; childIndex++)
      {
   
      Node_getChild(oNNode, childIndex, &oNChild);

      for(compareIndex = childIndex+1; compareIndex < children; compareIndex++){
         Node_getChild(oNNode, compareIndex, &twoChild);

         onePath = Node_getPath(oNChild);
         twoPath = Node_getPath(twoChild);

         if(Path_comparePath(onePath, twoPath) == 0){

            fprintf(stderr, "Duplicate nodes in the tree: \n");
            fprintf(stderr, "Node 1: %s \n", Path_getPathname(onePath));
            fprintf(stderr, "Node 2: %s \n", Path_getPathname(twoPath));
            
            return FALSE;
         }
      }
      }

   return TRUE;
}

/*
   Performs a pre-order traversal of the tree rooted at oNNode.
   Returns FALSE if a broken invariant is found and
   returns TRUE otherwise. Takes ulCount and nodeCount return FALSE if invariant is broken. 
*/


static boolean CheckerDT_treeCheck(Node_T oNNode, size_t ulCount, size_t* nodeCount) {


   size_t ulIndex;
   size_t numChildren;
   Path_T pathOne;
   Path_T pathTwo;
   int compare;

   assert(nodeCount != NULL);
   
   if(oNNode!= NULL) {
      (*nodeCount)++;
   
      /* Ensures no two children are the same */
      if(!CheckerDT_childCheck(oNNode)){
         return FALSE;
      }

      /* Sample check on each node: node must be valid */
      /* If not, pass that failure back up immediately */
      if(!CheckerDT_Node_isValid(oNNode))
         return FALSE;

      numChildren = Node_getNumChildren(oNNode);

      /* Recur on every child of oNNode.*/
      for(ulIndex = 0; ulIndex < numChildren; ulIndex++)
      {

         Node_T secondChild = NULL;
         Node_T oNChild = NULL;
         int iStatus = Node_getChild(oNNode, ulIndex, &oNChild);
      
         if(iStatus != SUCCESS) {
            fprintf(stderr, "getNumChildren claims more children than getChild returns\n");
            return FALSE;
         }

         /* Returns FALSE if children are not in lexicographical order */
         if(ulIndex < numChildren - 1){
            Node_getChild(oNNode, ulIndex + 1, &secondChild);
            pathOne = Node_getPath(oNChild);
            pathTwo = Node_getPath(secondChild);
            compare = Path_comparePath(pathOne, pathTwo);
      
            if(compare >= 0){
               fprintf(stderr, "First child: %s \n", Path_getPathname(pathOne));
               fprintf(stderr, "Second child: %s \n", Path_getPathname(pathTwo));
               fprintf(stderr, "Not in lexicographical order \n");
               return FALSE;
            }
         }

         /* if recurring down one subtree results in a failed check
            farther down, passes the failure back up immediately */
         if(!CheckerDT_treeCheck(oNChild, ulCount, nodeCount)){
            return FALSE;
      }
   }

   /* Compares ulCount with nodeCount once recursion ends at root*/
   if(Node_getParent(oNNode) == NULL){
      if(!CheckerDT_countCheck(ulCount, nodeCount)){
         return FALSE;
      }
   }
   }


   return TRUE;
}


/* see checkerDT.h for specification */
boolean CheckerDT_isValid(boolean bIsInitialized, Node_T oNRoot,
                          size_t ulCount){ 

   size_t nodeCount = 0;

   /* Sample check on a top-level data structure invariant:
      if the DT is not initialized, its count should be 0. */
   if(!bIsInitialized)
      if(ulCount != 0) {
         fprintf(stderr, "Not initialized, but count is not 0\n");
         return FALSE;
      }

   /* Now checks invariants recursively at each node from the root. */


   return CheckerDT_treeCheck(oNRoot, ulCount, &nodeCount);
}
