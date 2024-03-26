
/*--------------------------------------------------------------------*/
/* ft.c                                                               */
/* Author: Gabriel Marin & Kurt Lemai                                 */
/*--------------------------------------------------------------------*/

#include <stddef.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "dynarray.h"
#include "path.h"
#include "nodeFT.h"
#include "ft.h"


/*
  A File Tree is a representation of a hierarchy of directories and files, represented as an AO with 3 state variables:
*/

/* 1. a flag for being in an initialized state (TRUE) or not (FALSE) */
static boolean bIsInitialized;
/* 2. a pointer to the root node in the hierarchy */
static Node_T oNRoot;
/* 3. a counter of the number of nodes in the hierarchy */
static size_t ulCount;


/*
  Traverses the FT starting at the root as far as possible towards
  absolute path oPPath. If able to traverse, returns an int SUCCESS
  status and sets *poNFurthest to the furthest node reached (which may
  be only a prefix of oPPath, or even NULL if the root is NULL).
  Otherwise, sets *poNFurthest to NULL and returns with status:
  * CONFLICTING_PATH if the root's path is not a prefix of oPPath
  * MEMORY_ERROR if memory could not be allocated to complete request
*/

static int FT_traversePath(Path_T oPPath, Node_T *poNFurthest) {
  int iStatus;
  Path_T oPPrefix = NULL;
  Node_T oNCurr;
  Node_T oNChild = NULL;
  size_t ulDepth;
  size_t i;
  size_t ulChildID;

  assert(oPPath != NULL);
  assert(poNFurthest != NULL);

  /* root is NULL -> won't find anything */
  if(oNRoot == NULL) {
    *poNFurthest = NULL;
    return SUCCESS;
  }

  iStatus = Path_prefix(oPPath, 1, &oPPrefix);
  if(iStatus != SUCCESS) {
    *poNFurthest = NULL;
    return iStatus;
  }

  if(Path_comparePath(Node_getPath(oNRoot), oPPrefix)) {
    Path_free(oPPrefix);
    *poNFurthest = NULL;
    return CONFLICTING_PATH;
  
  }
  Path_free(oPPrefix);
  oPPrefix = NULL;

  oNCurr = oNRoot;
  ulDepth = Path_getDepth(oPPath);
  for(i = 2; i <= ulDepth; i++) {
    iStatus = Path_prefix(oPPath, i, &oPPrefix);
    if(iStatus != SUCCESS) {
      *poNFurthest = NULL;
      return iStatus;
    }

    if(Node_hasChild(oNCurr, oPPrefix, &ulChildID)) {
      /* go to that child and continue with next prefix */
      Path_free(oPPrefix);
      oPPrefix = NULL;
      iStatus = Node_getChild(oNCurr, ulChildID, &oNChild);
      if(iStatus != SUCCESS) {
        *poNFurthest = NULL;
         return iStatus;
      }

      oNCurr = oNChild;
    }
    else {
      /* oNCurr doesn't have child with path oPPrefix:
      this is as far as we can go */
      break;
    }
  }

  Path_free(oPPrefix);
  *poNFurthest = oNCurr;
  return SUCCESS;
}

/*
  Traverses the FT to find a node with absolute path pcPath. Returns a
  int SUCCESS status and sets *poNResult to be the node, if found.
  Otherwise, sets *poNResult to NULL and returns with status:
  * INITIALIZATION_ERROR if the FT is not in an initialized state
  * BAD_PATH if pcPath does not represent a well-formatted path
  * CONFLICTING_PATH if the root's path is not a prefix of pcPath
  * NO_SUCH_PATH if no node with pcPath exists in the hierarchy
  * MEMORY_ERROR if memory could not be allocated to complete request
 */

static int FT_findNode(const char *pcPath, Node_T *poNResult) {
  Path_T oPPath = NULL;
  Node_T oNFound = NULL;
  int iStatus;

  assert(pcPath != NULL);
  assert(poNResult != NULL);

  if(!bIsInitialized) {
    *poNResult = NULL;
    return INITIALIZATION_ERROR;
  }

  iStatus = Path_new(pcPath, &oPPath);
  if(iStatus != SUCCESS) {
    *poNResult = NULL;
    return iStatus;
  }

  iStatus = FT_traversePath(oPPath, &oNFound);
  if(iStatus != SUCCESS)
  {
    Path_free(oPPath);
    *poNResult = NULL;
    return iStatus;
  }

  if(oNFound == NULL) {
    Path_free(oPPath);
    *poNResult = NULL;
    return NO_SUCH_PATH;
  }

  if(Path_comparePath(Node_getPath(oNFound), oPPath) != 0) {
    Path_free(oPPath);
    *poNResult = NULL;
    return NO_SUCH_PATH;
  }

  Path_free(oPPath);
  *poNResult = oNFound;
  return SUCCESS;
}

int FT_init(void) {

  if(bIsInitialized)
    return INITIALIZATION_ERROR;

  bIsInitialized = TRUE;
  oNRoot = NULL;
  ulCount = 0;

  return SUCCESS;
}


/* Inserts a node of type file or directory with pcPath into the file tree with pvContents and the size of those contents ulLength. 
Returns SUCCESS if the new file is inserted successfully.
   Otherwise, returns:
   * INITIALIZATION_ERROR if the FT is not in an initialized state
   * BAD_PATH if pcPath does not represent a well-formatted path
   * CONFLICTING_PATH if the root exists but is not a prefix of pcPath,
                      or if the new file would be the FT root
   * NOT_A_DIRECTORY if a proper prefix of pcPath exists as a file
   * ALREADY_IN_TREE if pcPath is already in the FT (as dir or file)
   * MEMORY_ERROR if memory could not be allocated to complete request
*/

static int FT_insert(const char *pcPath, size_t type, void* pvContents, size_t ulLength){

  int iStatus;
  boolean ancestorIsFile; 
  boolean newPathIsFile;
  Path_T oPPath = NULL;
  Node_T oNFirstNew = NULL;
  Node_T oNCurr = NULL;
  size_t ulDepth, ulIndex;
  size_t ulNewNodes = 0;

  assert(pcPath != NULL);

  iStatus = Path_new(pcPath, &oPPath);
  if(iStatus != SUCCESS){
    return iStatus;
  }

  /* type of inserted node*/

  if(type == 0){
    newPathIsFile = FALSE;
  } else{
    newPathIsFile = TRUE;
  }


  /* find the closest ancestor of oPPath already in the tree */
  iStatus= FT_traversePath(oPPath, &oNCurr);  
  if(iStatus != SUCCESS)
  {
    Path_free(oPPath);
    return iStatus;
  }

  /* no ancestor node found, so if root is not NULL,
  pcPath isn't underneath root. */
  if(oNCurr == NULL && oNRoot != NULL) {
    Path_free(oPPath);
    return CONFLICTING_PATH;
  }

  /* Get's type of closest ancestor*/
  if(oNCurr != NULL){
  ancestorIsFile = Node_getType(oNCurr);
  }
  
  ulDepth = Path_getDepth(oPPath);


  /* file cannot be root */
  if(newPathIsFile){
    if(oNCurr == NULL && ulDepth == 1){
      Path_free(oPPath);
      return CONFLICTING_PATH;
    }
  }

  
  /*new root*/

  if(oNCurr == NULL){ 
    ulIndex = 1;
  }
  
  else {

    ulIndex = Path_getDepth(Node_getPath(oNCurr))+1;

    /* Cloest ancestor cannot be file*/
    if(ancestorIsFile == TRUE){
      Path_free(oPPath);
      return NOT_A_DIRECTORY;
    }

    /* oNCurr is the node we're trying to insert */
    if(ulIndex == ulDepth+1 && !Path_comparePath(oPPath, Node_getPath(oNCurr))){
      Path_free(oPPath);
      return ALREADY_IN_TREE;
    }
  }

  /* starting at oNCurr, build rest of the path one level at a time */

  while(ulIndex <= ulDepth) {

    Path_T oPPrefix = NULL;
    Node_T oNNewNode = NULL;

    /* generate a Path_T for this level */
    iStatus = Path_prefix(oPPath, ulIndex, &oPPrefix);
    if(iStatus != SUCCESS) {

      Path_free(oPPath);

      if(oNFirstNew != NULL){
        (void) Node_free(oNFirstNew);
      }

      return iStatus;
    }

    /* insert the new node for this level */

    if(newPathIsFile && ulIndex == ulDepth){
      iStatus = Node_new(oPPrefix, oNCurr, &oNNewNode, 1, pvContents, ulLength); 
    } else{
      iStatus = Node_new(oPPrefix, oNCurr, &oNNewNode, 0, NULL, 0); 
    }


    if(iStatus != SUCCESS) {
      Path_free(oPPath);
      Path_free(oPPrefix);

      if(oNFirstNew != NULL){
        (void) Node_free(oNFirstNew); /* Modify */
      }

      return iStatus;
    }

    /* set up for next level */
    Path_free(oPPrefix);
    oNCurr = oNNewNode;
    ulNewNodes++;
    if(oNFirstNew == NULL){
      oNFirstNew = oNCurr;
    }
    ulIndex++;
  }

  Path_free(oPPath);
  /* update DT state variables to reflect insertion */
  if(oNRoot == NULL){
    oNRoot = oNFirstNew;
  }

  ulCount += ulNewNodes;
  return SUCCESS;
}


int FT_insertDir(const char*pcPath){

  assert(pcPath != NULL);

  /* validate pcPath and generate a Path_T for it */
  if(!bIsInitialized){
    return INITIALIZATION_ERROR;
  }


  return FT_insert(pcPath, 0, NULL, 0);
}

int FT_insertFile(const char*pcPath, void *pvContents, size_t ulLength){
  assert(pcPath != NULL);
  

  /* validate pcPath and generate a Path_T for it */
  if(!bIsInitialized){
    return INITIALIZATION_ERROR;
  }

  return FT_insert(pcPath, 1, pvContents, ulLength);
}


/*
  Performs a pre-order traversal of the tree rooted at oNNode,
  inserting each payload to DynArray_T dynArray beginning at index uIndex. Returns the next unused index in d after the insertion(s).
*/

static size_t FT_preOrderTraversal(Node_T oNNode, DynArray_T dynArray, size_t uIndex) {

  size_t c;

  assert(dynArray != NULL);

  if(oNNode != NULL) {
    (void) DynArray_set(dynArray, uIndex, oNNode);
    uIndex++;

    for(c = 0; c < Node_getNumChildren(oNNode); c++) {
      int iStatus;
      Node_T oNChild = NULL;
      iStatus = Node_getChild(oNNode,c, &oNChild);
      assert(iStatus == SUCCESS);
      if(Node_getType(oNChild) == TRUE) {
        (void) DynArray_set(dynArray, uIndex, oNChild);
        uIndex++;
      }
    }

    for(c = 0; c < Node_getNumChildren(oNNode); c++) {
      int iStatus;
      Node_T oNChild = NULL;
      iStatus = Node_getChild(oNNode, c, &oNChild);
      assert(iStatus == SUCCESS);
      if(Node_getType(oNChild) == FALSE) {
      uIndex = FT_preOrderTraversal(oNChild, dynArray, uIndex);
      }
    }
  }

  return uIndex;
}

/*
  Alternate version of strlen that uses pulAcc as an in-out parameter
  to accumulate a string length, rather than returning the length of
  oNNode's path, and also always adds one addition byte to the sum.
*/
static void FT_strlenAccumulate(Node_T oNNode, size_t *pulAcc) {
   assert(pulAcc != NULL);

   if(oNNode != NULL)
      *pulAcc += (Path_getStrLength(Node_getPath(oNNode)) + 1);
}

/*
  Alternate version of strcat that inverts the typical argument
  order, appending oNNode's path onto pcAcc, and also always adds one
  newline at the end of the concatenated string.
*/
static void FT_strcatAccumulate(Node_T oNNode, char *pcAcc) {
   assert(pcAcc != NULL);

   if(oNNode != NULL) {
      strcat(pcAcc, Path_getPathname(Node_getPath(oNNode)));
      strcat(pcAcc, "\n");
   }
}

char *FT_toString(void) {

  DynArray_T nodes;
  size_t totalStrlen = 1;
  char *result = NULL;

  if(!bIsInitialized)
    return NULL;

  nodes = DynArray_new(ulCount);
  (void) FT_preOrderTraversal(oNRoot, nodes, 0);

  DynArray_map(nodes, (void (*)(void *, void*)) FT_strlenAccumulate,
  (void*) &totalStrlen);

  result = malloc(totalStrlen);
  if(result == NULL) {
    DynArray_free(nodes);
    return NULL;
  }
  *result = '\0';

  DynArray_map(nodes, (void (*)(void *, void*)) FT_strcatAccumulate,
              (void *) result);

  DynArray_free(nodes);

  return result;
}

boolean FT_containsDir(const char *pcPath){

  int iStatus;
  boolean type;

  Node_T oNFound = NULL;

  assert(pcPath != NULL);

  iStatus = FT_findNode(pcPath, &oNFound);
   
  if(iStatus != SUCCESS){
    return FALSE;
  }

  type = Node_getType(oNFound);

  /* Found node has to be a directory*/
  if(type == FALSE && iStatus == 0){
   return TRUE;
   }

   return FALSE;
}


boolean FT_containsFile(const char *pcPath){

  int iStatus;
  boolean type;
  Node_T oNFound = NULL;

  assert(pcPath != NULL);

  iStatus = FT_findNode(pcPath, &oNFound);

  if(iStatus != SUCCESS){
    return FALSE;
  }
  type = Node_getType(oNFound);
   
  /* Found node has to be a file*/
  if(type == TRUE && iStatus == 0){
    return TRUE;
  }

  return FALSE;

}


int FT_rmDir(const char *pcPath){
  
  int iStatus;
  Node_T oNFound = NULL;

  assert(pcPath != NULL);

  iStatus = FT_findNode(pcPath, &oNFound);

  if(iStatus != SUCCESS){
    return iStatus;
  }

  /* Found node must be directory*/
  if(Node_getType(oNFound) == FALSE){

    ulCount -= Node_free(oNFound);
    if(ulCount == 0){
      oNRoot = NULL;
    }

    return SUCCESS;
  }

  else {
    return NOT_A_DIRECTORY;
  }
}

int FT_rmFile(const char *pcPath) {
  int iStatus;
  Node_T oNFound = NULL;

  assert(pcPath != NULL);
  iStatus = FT_findNode(pcPath, &oNFound);

  if(iStatus != SUCCESS){
    return iStatus;
  }

  if(Node_getType(oNFound) == TRUE)
  {
    ulCount -= Node_free(oNFound);
    if(ulCount == 0){
      oNRoot = NULL;
    }
      return SUCCESS;
  }
  else {
    return NOT_A_FILE;
  }
}

void *FT_getFileContents(const char *pcPath){

  Node_T oNFound = NULL;
  boolean isFile;
  void *pvContents;
  int iStatus;

  if(pcPath == NULL){
    return NULL;
  }

  iStatus = FT_findNode(pcPath, &oNFound);
  if(iStatus != SUCCESS){
    return NULL;
  }

  isFile = Node_getType(oNFound);

  if(!isFile){
    return NULL;
  }

  pvContents = Node_getFileContents(oNFound);
  return pvContents;
}


int FT_destroy(void) {

  if(!bIsInitialized){
    return INITIALIZATION_ERROR;
  }

  if(oNRoot) {
    ulCount -= Node_free(oNRoot);
    oNRoot = NULL;
  }

  bIsInitialized = FALSE;

  return SUCCESS;
}


int FT_stat(const char *pcPath, boolean *pbIsFile, size_t *pulSize){

  Node_T oNFound = NULL;
  boolean isFile;
  int iStatus;
  size_t byteSize;

  assert(pbIsFile != NULL);
  assert(pulSize != NULL);

  if(!bIsInitialized){
    return INITIALIZATION_ERROR;
  }  
  
  if(pcPath == NULL){
    return BAD_PATH;
  }

  iStatus = FT_findNode(pcPath, &oNFound);

  if(iStatus != SUCCESS){
    return iStatus;
  }

  isFile = Node_getType(oNFound);
  byteSize = Node_getFileSize(oNFound);

  if(isFile){
    *pbIsFile = TRUE;
    *pulSize = byteSize;
    return iStatus;
  }

  else{
    *pbIsFile = FALSE;
    return iStatus;
  }

}


void *FT_replaceFileContents(const char *pcPath, void *pvNewContents,
size_t ulNewLength){

  Node_T oNFound = NULL;
  boolean isFile;
  void *pvContents;
  int iStatus;

  if(pcPath == NULL){
    return NULL;
  }

  iStatus = FT_findNode(pcPath, &oNFound);
  if(iStatus != SUCCESS){
    return NULL;
  }

  isFile = Node_getType(oNFound);

  if(!isFile){
    return NULL;
  }

  pvContents = Node_getFileContents(oNFound);
  Node_replaceFileContents(oNFound, pvNewContents, ulNewLength);

  return pvContents;
}
