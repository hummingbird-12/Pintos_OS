#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "main.h"

#define ASSERT(CONDITION) assert(CONDITION)

int main(){  

  char command[COMMAND_MAX_SIZE];
  char para[PARA_SIZE][COMMAND_MAX_SIZE];
  char ds[COMMAND_MAX_SIZE];

  dc_head=NULL;
  dc_tail=NULL;


  while(1){
    command_init(command,ds,para);
    command_input(command, ds, para);
    //  printf("test: %s/%s/%s\n",command,ds,para[0]);
    if(command_process(command, ds, para)==-1)  break;


  }


}

void command_init(char command[], char ds[], char para[][COMMAND_MAX_SIZE]){
  int i;

  memset(command, '\0', COMMAND_MAX_SIZE);
  memset(ds, '\0', COMMAND_MAX_SIZE);
  for(i=0; i<PARA_SIZE ; i++){
    memset(para[i], '\0', COMMAND_MAX_SIZE);
  }
}

void command_input(char command[], char ds[], char para[][COMMAND_MAX_SIZE]){
  int cnt=0;
  char *token;
  char input[COMMAND_MAX_SIZE];

  fgets(input,COMMAND_MAX_SIZE,stdin);
  token = strtok(input, " \n");
  do{
    if(cnt==0) strcpy(command, token);
    else if(cnt==1) strcpy(ds, token);
    else strcpy( para[cnt-2] ,token);
    cnt++;
  }while (token = strtok(NULL, " \n"));  

}


int command_process(char command[], char ds[], char para[][COMMAND_MAX_SIZE]){
  int i;
  for(i=0 ; i<ETC_NUM ; i++)
    if( !strcmp(str_cmd_etc[i],command) ){
      if(i==QUIT) return -1;
      else etc_process(i,ds,para);
      return 0;
    }
  for(i=0 ; i<LIST_NUM ; i++)
    if(!strcmp(str_cmd_list[i],command) ){
      list_process(i,ds,para);
      return 0;
    }

  for(i=0; i<HASH_NUM ; i++)
    if(!strcmp(str_cmd_hash[i], command)){
      hash_process(i, ds, para);
      return 0;
    }

  for(i=0; i<BITMAP_NUM ; i++)
    if(!strcmp(str_cmd_bitmap[i], command)){
      bitmap_process(i, ds, para);
      return 0;
    }
  ASSERT(1);
}
/////////////////////////////// ETC ///////////////////////////////////
void etc_process(int cmd,char ds[], char para[][COMMAND_MAX_SIZE]){
  switch(cmd){

    case CREATE:
      create(ds,para);
      break;

    case DELETE:
      strcpy(para[0],ds);
      delete(para);
      break;

    case DUMPDATA:      
      strcpy(para[0],ds);
      dumpdata(para);
      break;

  }

}

void create(char ds[], char para[][COMMAND_MAX_SIZE]){
  int i=0;
  struct data_collect *dc;
  struct list *list;
  struct hash *hash;
  struct bitmap *bitmap;
  size_t bit_num;

  dc=(struct data_collect*)malloc(sizeof(struct data_collect));


  for(i=0; i<DS_NUM; i++)
    if(!strcmp(str_ds[i],ds)){
      switch(i){


        case LIST:
          list=(struct list*)malloc(sizeof(struct list));
          (dc->data).list = list;
          list_init(list);
          break;


        case HASHTABLE:
          hash=(struct hash*)malloc(sizeof(struct hash));
          (dc->data).hash = hash;
          hash_init(hash, hash_int_func,hash_less,NULL);
          break;

        case BITMAP:
          bit_num = (size_t)strtol(para[1],NULL,10);
          bitmap = bitmap_create(bit_num);
          (dc->data).bitmap = bitmap;
          break;
      }
      break;
    }

  //make and link data_collect 
  strcpy(dc->name,para[0]);
  dc->next = NULL;
  dc->data_type = i;
  if(dc_head == NULL){
    dc_head = dc;
    dc_tail = dc;
  }
  else{
    dc_tail->next = dc;
    dc_tail=dc;
  }


}

void delete(char para[][COMMAND_MAX_SIZE]){
  struct data_collect *dc, *bef_dc;

  struct list *list;
  struct list_elem *elem, *del_elem;

  struct hash *hash;
  struct hash_iterator iter;

  struct bitmap *bitmap;
  dc = find_data_collect(para[0]);
  ASSERT(dc!=NULL);

  switch(dc->data_type){
    case LIST:
      list = (dc->data).list;
      for(elem = list_begin(list) ; elem != list_end(list);){
        del_elem = elem;
        if(elem != list_end(list))
          break;
        elem = list_next(elem);
        free(del_elem);
      }
      break;
    case HASHTABLE:
      hash = (dc->data).hash;
      hash_first(&iter, hash);

      while(hash_next(&iter)){
        hash_delete(hash, hash_cur(&iter));
      }
      break;

    case BITMAP: 
      bitmap = (dc->data).bitmap;
      bitmap_destroy(bitmap);
      break;
  }

  if(dc == dc_head){
    dc_head = dc->next;
    if(dc == dc_tail)
      dc_tail = NULL;
  }
  else{
    for(bef_dc = dc_head; bef_dc->next != dc; bef_dc=bef_dc->next) break;
    bef_dc->next=dc->next;
    if(dc == dc_tail)
      dc_tail=bef_dc;
  }

  free(dc);

}

void dumpdata(char para[][COMMAND_MAX_SIZE]){
  //searching data
  struct data_collect *dc;
  struct list *list;
  struct list_elem *elem;

  struct hash *hash;
  struct hash_iterator iter;
  struct hash_elem *h_elem;

  struct bitmap *bitmap;
  int bit;
  dc = find_data_collect(para[0]);
  ASSERT(dc!=NULL);
  switch(dc->data_type){
    case LIST:
      list=(dc->data).list;
      if(list_empty(list)){
        return;
      }
      for(elem = list_begin(list) ; elem != list_end(list); elem = list_next(elem)){
        printf("%d ",(list_entry(elem,struct list_item, elem) )->data);
      }
      printf("\n");
      break;

    case HASHTABLE:
      hash = (dc->data).hash;
      if(hash_empty(hash))  return;

      hash_first(&iter, hash);
      while(hash_next(&iter)){ 
        printf("%d ",hash_entry( hash_cur(&iter) , struct hash_item, elem)->data);
      }
      printf("\n");
      break;

    case BITMAP:
      bitmap = (dc->data).bitmap;
      if(bitmap_size(bitmap)==0) break;
      for(bit = 0 ; bit<bitmap_size(bitmap); bit++){
        printf("%d",bitmap_test(bitmap,bit));
      }
      printf("\n");
      break;
  }
}

struct data_collect* find_data_collect(char name[]){
  struct data_collect *dc;
  for(dc = dc_head ; dc!=NULL; dc=dc->next){
    if(!strcmp(dc->name, name)) return dc;
  }
  return NULL;
}

//////////////////// LIST ////////////////////////////

void list_process(int cmd, char ds[], char para[][COMMAND_MAX_SIZE] ){
  struct data_collect *dc;
  struct list_item *item;
  struct list_elem *elem,*elem1,*elem2,*elem_tmp;
  struct list *list,*list1,*list2;
  int cnt;
  int idx1,idx2,idx3;

  switch(cmd){
    case LIST_INSERT:
      dc = find_data_collect(ds);
      ASSERT(dc != NULL);

      item = (struct list_item*)malloc(sizeof(struct list_item));
      item->data = strtol(para[1],NULL,10);
      cnt = strtol(para[0],NULL,10);

      list = (dc->data).list;
      for(elem = list_begin(list) ; elem != list_end(list);){
        if(cnt==0) break; 
        elem = list_next(elem);
        cnt--;
      }

      list_insert(elem,&(item->elem));
      break;


    case LIST_SPLICE:
      dc = find_data_collect(ds);
      ASSERT(dc != NULL);
      list1 = (dc->data).list;

      cnt = 0;
      idx1 = strtol(para[0],NULL,10);

      dc = find_data_collect(para[1]);
      ASSERT(dc != NULL);
      list2 = (dc->data).list;

      idx2 = strtol(para[2],NULL,10);
      idx3 = strtol(para[3],NULL,10);

      for(elem = list_begin(list1) ; elem != list_end(list1) ; ){
        if(cnt == idx1) break;
        elem  = list_next(elem);
        cnt++;
      }

      cnt=0;
      for(elem_tmp = list_begin(list2); elem_tmp!= list_end(list2);){
        if(cnt == idx3){
          elem2 = elem_tmp;
          break;
        }
        if(cnt == idx2) elem1 = elem_tmp;

        elem_tmp = list_next(elem_tmp);
        cnt++;
      }
      list_splice(elem, elem1,elem2);
      break;

    case LIST_PUSH_FRONT:
      dc = find_data_collect(ds);
      ASSERT(dc != NULL); 
      item = (struct list_item*)malloc(sizeof(struct list_item));
      item->data = strtol(para[0],NULL,10);
      list_push_front((dc->data).list, &(item->elem));
      break;

    case LIST_PUSH_BACK:
      dc = find_data_collect(ds);
      ASSERT(dc != NULL); 
      item = (struct list_item*)malloc(sizeof(struct list_item));
      item->data = strtol(para[0],NULL,10);
      list_push_back((dc->data).list, &(item->elem));
      break;

    case LIST_REMOVE:
      dc = find_data_collect(ds);
      ASSERT(dc != NULL);

      cnt = 0;
      idx1 = strtol(para[0], NULL , 10);

      list = (dc->data).list;
      for(elem = list_begin(list) ; elem != list_end(list) ; ){
        if(cnt == idx1)
          break;
        elem = list_next(elem);
        cnt++;
      }
      list_remove(elem);
      break;
    case LIST_POP_FRONT:
      dc = find_data_collect(ds);
      ASSERT(dc != NULL);

      list = (dc->data).list;
      list_pop_front(list);
      break;
    case LIST_POP_BACK:

      dc = find_data_collect(ds);
      ASSERT(dc != NULL);

      list = (dc->data).list;
      list_pop_back(list);
      break;
    case LIST_FRONT:
      dc = find_data_collect(ds);
      ASSERT(dc != NULL);

      list = (dc->data).list;
      elem = list_front(list);
      printf("%d\n",(list_entry(elem,struct list_item, elem) )->data);

      break;
    case LIST_BACK:   
      dc = find_data_collect(ds);
      ASSERT(dc != NULL);

      list = (dc->data).list;
      elem = list_back(list);
      printf("%d\n",(list_entry(elem,struct list_item, elem) )->data);


      break;
    case LIST_SIZE:  
      dc = find_data_collect(ds);
      ASSERT(dc != NULL);

      list = (dc->data).list;
      printf("%zu\n",list_size(list));
      break;
    case LIST_EMPTY:
      dc = find_data_collect(ds);
      ASSERT(dc != NULL);

      list = (dc->data).list;
      if(list_empty(list)==true) printf("true\n");
      else printf("false\n");

      break;
    case LIST_REVERSE:
      dc = find_data_collect(ds);
      ASSERT(dc != NULL);

      list = (dc->data).list;
      list_reverse(list);
      break;
    case LIST_SORT:
      dc = find_data_collect(ds);
      ASSERT(dc != NULL);

      list = (dc->data).list;
      list_sort(list,list_less,NULL);
      break;
    case LIST_INSERT_ORDERED:
      dc = find_data_collect(ds);
      ASSERT(dc != NULL);

      list = (dc->data).list;
      item = (struct list_item*)malloc(sizeof(struct list_item));
      item->data = strtol(para[0],NULL,10);
      list_insert_ordered(list,&(item->elem),list_less,NULL );
      break;
    case LIST_UNIQUE:
      dc = find_data_collect(ds);
      ASSERT(dc != NULL);

      list = (dc->data).list;

      if(para[0][0] =='\0'){
        list_unique(list,NULL,list_less,NULL);
        break;
      }
      dc = find_data_collect(para[0]);
      list1 = (dc->data).list;
      list_unique(list,list1,list_less,NULL);
      break;

    case LIST_MAX:
      dc = find_data_collect(ds);
      ASSERT(dc != NULL);

      list = (dc->data).list;
      elem = list_max(list,list_less,NULL);
      printf("%d\n",(list_entry(elem,struct list_item, elem) )->data);
      break;
    case LIST_MIN:
      dc = find_data_collect(ds);
      ASSERT(dc != NULL);

      list = (dc->data).list;
      elem = list_min(list,list_less,NULL);
      printf("%d\n",(list_entry(elem,struct list_item, elem) )->data);
      break;

    case LIST_SWAP:

      dc = find_data_collect(ds);
      ASSERT(dc != NULL);

      idx1 = strtol(para[0], NULL , 10);
      idx2 = strtol(para[1], NULL , 10);
      list = (dc->data).list; 
      cnt=0;
      for(elem = list_begin(list) ; elem != list_end(list) ; ){
        if(cnt == idx1) elem1 = elem;
        if(cnt == idx2) elem2 = elem;
        elem = list_next(elem);
        cnt++;
      }
      list_swap(elem1,elem2);
      break;

    case LIST_SHUFFLE:
      dc = find_data_collect(ds);
      ASSERT(dc != NULL);

      list = (dc->data).list;
      list_shuffle(list);
      break;
  }
}

static bool list_less(const struct list_elem *a,const struct list_elem *b,void *aux){

  if(list_entry(a,struct list_item, elem)->data <list_entry(b,struct list_item, elem)->data) 
    return true;
  else
    return false;

}



////////// HASH ///////////////

void hash_process(int cmd,char ds[], char para[][COMMAND_MAX_SIZE]){
  struct data_collect *dc;
  struct hash* hash;
  struct hash_iterator iter;
  struct hash_elem* elem;
  struct hash_item* item; 

  dc = find_data_collect(ds);
  ASSERT(dc != NULL);

  switch(cmd){
    case HASH_INSERT:
      hash = (dc->data).hash;
      item = (struct hash_item*)malloc(sizeof(struct hash_item));
      item->data = strtol(para[0],NULL,10);

      hash_insert(hash,&(item->elem));
      break;

    case HASH_REPLACE:
      hash = (dc->data).hash;

      item = (struct hash_item*)malloc(sizeof(struct hash_item));
      item->data = strtol(para[0],NULL,10);
      hash_replace(hash, &(item->elem));
      break;

    case HASH_FIND:
      hash = (dc->data).hash;

      item = (struct hash_item*)malloc(sizeof(struct hash_item));
      item->data = strtol(para[0],NULL,10); 
      elem = hash_find(hash,&(item->elem));
      if(elem==NULL) break;

      printf("%d\n",hash_entry(elem,struct hash_item,elem)->data);
      
      break;

    case HASH_DELETE:
      hash = (dc->data).hash;
      item = (struct hash_item*)malloc(sizeof(struct hash_item));
      item->data = strtol(para[0],NULL,10); 

      hash_delete(hash,&(item->elem));
      
      break;

    case HASH_CLEAR:
      hash = (dc->data).hash;
      hash_clear(hash, hash_destructor);
      break;

    case HASH_SIZE:
      hash = (dc->data).hash;
      printf("%zu\n",hash_size(hash));
      break;

    case HASH_EMPTY:
      hash = (dc->data).hash;
      if(hash_empty(hash)) printf("true\n");
      else printf("false\n");
      break;

    case HASH_APPLY:
      hash = (dc->data).hash;
      if(!strcmp(para[0],"triple") )
        hash_apply(hash,hash_triple);
      else
        hash_apply(hash,hash_square);

      break;

  }


}

bool hash_less(const struct hash_elem *a, const struct hash_elem *b, void *aux){

  if(hash_entry(a,struct hash_item, elem)->data < hash_entry(b,struct hash_item, elem)->data) 
    return true;
  else
    return false;


}
unsigned hash_int_2_func(const struct hash_elem *elem, void *aux){
  struct hash_item *item;

  item = hash_entry(elem,struct hash_item, elem);
  return hash_int_2(item->data);
}

void hash_destructor(struct hash_elem *elem,void *aux){
  struct hash_item *item;
  item = hash_entry(elem, struct hash_item, elem);
  item = NULL;
}

void hash_square(struct hash_elem *elem,void *aux){
  struct hash_item *item;
  item = hash_entry(elem, struct hash_item, elem);
  item->data *= item->data;
}
void hash_triple(struct hash_elem *elem,void *aux){
  struct hash_item *item;
  item = hash_entry(elem, struct hash_item, elem);
  item->data *= (item->data*item->data);
}

unsigned hash_int_func(const struct hash_elem *elem, void *aux){
  struct hash_item *item;

  item = hash_entry(elem,struct hash_item, elem);
  return hash_int(item->data);
}



////////////// bitmap ///////////////////

void bitmap_process(int cmd, char ds[], char para[][COMMAND_MAX_SIZE]){
  struct data_collect* dc;
  struct bitmap* bitmap;
  int bit; 
  dc = find_data_collect(ds);
  ASSERT(dc != NULL);
  bitmap = (dc->data).bitmap;
  //printf("::%d %d\n",cmd,)
  switch(cmd){
    case BITMAP_SIZE:
      printf("%zu\n",bitmap_size(bitmap));
      break;
    case BITMAP_SET:
      bitmap_set(bitmap,(size_t)strtol(para[0],NULL,10),!strcmp("true",para[1]));
      break;
    case BITMAP_MARK:
      bitmap_mark(bitmap,(size_t)strtol(para[0],NULL,10));
      break;
    case BITMAP_RESET:
      bitmap_reset(bitmap,(size_t)strtol(para[0],NULL,10));
      break;
    case BITMAP_FLIP:
      bitmap_flip(bitmap,(size_t)strtol(para[0],NULL,10));
      break;
    case BITMAP_TEST:
      if( bitmap_test(bitmap,(size_t)strtol(para[0],NULL,10)) )
        printf("true\n");
      else printf("false\n");
      break;
    case BITMAP_SET_ALL:
      bitmap_set_all(bitmap,!strcmp(para[0],"true"));
      break;

    case BITMAP_SET_MULTIPLE:
      bitmap_set_multiple(bitmap,(size_t)strtol(para[0],NULL,10) ,(size_t)strtol(para[1],NULL,10),!strcmp(para[2],"true"));
      break;

    case BITMAP_COUNT:
      printf("%zu\n",bitmap_count(bitmap,(size_t)strtol(para[0],NULL,10) ,(size_t)strtol(para[1],NULL,10),!strcmp(para[2],"true")));
      break;

    case BITMAP_CONTAINS:
      if(bitmap_contains( bitmap, (size_t)strtol(para[0],NULL,10) ,(size_t)strtol(para[1],NULL,10),!strcmp(para[2],"true")))
        printf("true\n");
      else
        printf("false\n");
      break;

    case BITMAP_ANY:
      if( bitmap_any(bitmap,(size_t)strtol(para[0],NULL,10),(size_t)strtol(para[1],NULL,10)) )
        printf("true\n");
      else 
        printf("false\n");
      break;
    case BITMAP_NONE:
      if( bitmap_none(bitmap,(size_t)strtol(para[0],NULL,10),(size_t)strtol(para[1],NULL,10)) )
        printf("true\n");
      else 
        printf("false\n");
      break;

    case BITMAP_ALL:
      if( bitmap_all(bitmap,(size_t)strtol(para[0],NULL,10),(size_t)strtol(para[1],NULL,10)) )
        printf("true\n");
      else 
        printf("false\n");
      break;

    case BITMAP_DUMP:
      bitmap_dump(bitmap);
      break;



    case BITMAP_SCAN_AND_FLIP:
      printf("%u\n", bitmap_scan_and_flip(bitmap, (size_t)strtol(para[0],NULL,10) ,(size_t)strtol(para[1],NULL,10), !strcmp(para[2],"true")) );
      break;

    case BITMAP_SCAN:
      printf("%u\n", bitmap_scan(bitmap,(size_t)strtol(para[0],NULL,10) ,(size_t)strtol(para[1],NULL,10), !strcmp(para[2],"true")) );
      break;



    case BITMAP_EXPAND:
      bitmap_expand(bitmap,(size_t)strtol(para[0],NULL,10)); 
      break;

  }
}