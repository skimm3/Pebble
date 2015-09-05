#include <pebble.h>

static Window *window;
static TextLayer *text_layer;
static Layer* layer;

const uint32_t HIGH_SCORE_KEY = 1335;
const uint32_t WON_ONCE_KEY = 1336;

static GBitmap *spades_image[13];
static GBitmap *clubs_image[13];
static GBitmap *diamonds_image[13];
static GBitmap *hearts_image[13];
static GBitmap *card_back_image;
static GBitmap *pointer_image;

/*
Deck Info
Color order: Spades, Diamonds, Hearts, Clubs
Faces: {'A','2','3','4','5','6','7','8','9','X','J','Q','K'}

0 = Ace of spades
13 = Ace of Diamonds

Calc card: 
Color = number/13
Value = number%13

*/

int deck[52];
int table[4][13];
int top_card_index = 0;
int card_top[4];
int selected;
int high_score = 52;
bool won_round = false;
bool won_once = false;


typedef enum {DECK, CARD_1, CARD_2, CARD_3, CARD_4} pointer_location;
pointer_location pointer = CARD_1;

static void deal();
static void print_card();
static void restart();

static bool higher_value(int card_1, int card_2){
  if (card_1/13 == card_2/13){
   // APP_LOG(APP_LOG_LEVEL_DEBUG, "card1=%i, card2=%i, %i", card_1,card_2, card_1 % 13);
    return card_2 % 13 == 0 ? true: (card_1 % 13 == 0 ? false: card_1<card_2);
  }
  return false;
}

static bool valid_move(pointer_location pointer){
  int cur_card = 0;
  switch(pointer){
    case DECK :      
      return false;
    case CARD_1 :
      cur_card = 0;
      break;
    case CARD_2 :
      cur_card = 1;
      break;
    case CARD_3 :
      cur_card = 2;
      break;
    case CARD_4 :
      cur_card = 3;     
      break;
  }
  if (card_top[cur_card] == -1){
    return false;
  }

  int i;
  for(i = 0; i < 4; ++i){
    if (i != cur_card && card_top[cur_card] != -1){
      if (higher_value(table[cur_card][card_top[cur_card]], table[i][card_top[i]])){
        return true;
      }
    }    
  }  
  return false;
}  

static bool remove_card(pointer_location pointer){
  if (valid_move(pointer)){
    switch(pointer){
      case DECK :      
        break;      
      case CARD_1 :
        if (card_top[0] != -1) { card_top[0]--; } 
        break;
      case CARD_2 :
        if (card_top[1] != -1) { card_top[1]--; } 
        break;
      case CARD_3 :
        if (card_top[2] != -1) { card_top[2]--; } 
        break;
      case CARD_4 :
        if (card_top[3] != -1) { card_top[3]--; } 
        break;    
    }
    return true;
  }
  return false;
}

static bool free_slot(){
  return (card_top[0] == -1 || card_top[1] == -1 || card_top[2] == -1 || card_top[3] == -1);
}

static void select_card(){
  int cur_card = 0;
  switch(pointer){
    case DECK :      
      break;
    case CARD_1 :
      cur_card = 0;
      break;
    case CARD_2 :
      cur_card = 1;
      break;
    case CARD_3 :
      cur_card = 2;
      break;
    case CARD_4 :
      cur_card = 3;     
      break;
  }
  if(card_top[cur_card] != -1 && selected == -1 && free_slot()){
    selected = cur_card;    
  }
  else{
    selected = -1;
  }
}

static void move_card(){
  if(selected != -1){
    
    switch(pointer){
      case DECK :
        break;
      case CARD_1 :
        if (selected != 0 && card_top[0] == -1){
          card_top[0]++;
          table[0][card_top[0]] = table[selected][card_top[selected]];
          card_top[selected]--;
        }
        break;
      case CARD_2 :
        if (selected != 1 && card_top[1] == -1){
          card_top[1]++;
          table[1][card_top[1]] = table[selected][card_top[selected]];
          card_top[selected]--;
        }
        break;
      case CARD_3 :
        if (selected != 2 && card_top[2] == -1){
          card_top[2]++;
          table[2][card_top[2]] = table[selected][card_top[selected]];
          card_top[selected]--;
        }
        break;
      case CARD_4 :
        if (selected != 3 && card_top[3] == -1){
          card_top[3]++;
          table[3][card_top[3]] = table[selected][card_top[selected]];
          card_top[selected]--;
        }     
        break;
    }    
  }
}

static void check_win_condition(){
  int cards_left = 4 + card_top[0]+card_top[1]+card_top[2]+card_top[3]+(52-top_card_index);
  if (top_card_index == 52 && card_top[0] == 0 && card_top[1] == 0 && card_top[2] == 0 && card_top[3] == 0 && !won_round){
    if (!won_once){
          won_once = true;
          high_score = 1; 
          won_round = true;
    }
    else{
      high_score++;
      won_round = true;
    }    
  }
  else if (cards_left < high_score && !won_once){
    high_score = cards_left;
  }
}

static void long_select_click_handler(ClickRecognizerRef recognizer, void *context) {
  if (top_card_index != 52) {
    deal();
  } else {
    restart();
  }
  check_win_condition();
  layer_mark_dirty(layer);
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  if (pointer == DECK){
    if (top_card_index != 52){ 
      deal();
    }
    else{
      restart();
    }
  }
  else if(!remove_card(pointer) || selected != -1){
    move_card();
    select_card();
  }
  check_win_condition();
  layer_mark_dirty(layer);
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  switch(pointer){
    case DECK :
      pointer = CARD_4;
      break;
    case CARD_1 :
      pointer = DECK;
      break;
    case CARD_2 :
      pointer = CARD_1;
      break;
    case CARD_3 :
      pointer = CARD_2;
      break;
    case CARD_4 :
      pointer = CARD_3;
      break;    
  }
  layer_mark_dirty(layer);
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  switch(pointer){
    case DECK :
      pointer = CARD_1;
      break;
    case CARD_1 :
      pointer = CARD_2;
      break;
    case CARD_2 :
      pointer = CARD_3;
      break;
    case CARD_3 :
      pointer = CARD_4;
      break;
    case CARD_4 :
      pointer = DECK;
      break;    
  }
  layer_mark_dirty(layer);
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
  window_long_click_subscribe(BUTTON_ID_SELECT, 1000, long_select_click_handler, NULL);
}

static GBitmap* card_image_from_index(int card_index){
  int color = card_index/13; 
  int value = card_index % 13;
  switch(color){
    case 0 :
      return spades_image[value];
    case 1 :
      return diamonds_image[value];
    case 2 :
      return hearts_image[value];
    case 3 :
      return clubs_image[value];
    default :
      return NULL;
  }  
}

static void layer_update_callback(Layer* this_layer, GContext* ctx){
  int card_width = 32;
  int card_height = 48;
  int card_1_xpos = 5;
  int card_1_ypos = 70;
  int card_space = 2;
  int deck_xpos = 5;
  int deck_ypos = 11;

  //Draw background
  #ifdef PBL_COLOR
      graphics_context_set_compositing_mode(ctx, GCompOpSet);
      graphics_context_set_fill_color(ctx, GColorIslamicGreen);
  #elif PBL_BW
      graphics_context_set_compositing_mode(ctx, GCompOpAssign);
      graphics_context_set_fill_color(ctx, GColorWhite);
  #endif
  graphics_fill_rect(ctx, (GRect) { .origin = { 0, 0 }, .size = { 144, 168 }}, 0, GCornerNone);
  
  //Draw deck
  if (top_card_index != 52){
    graphics_draw_bitmap_in_rect(ctx, card_back_image, GRect(deck_xpos,deck_ypos,card_width,card_height));
  }
  else{
    #ifdef PBL_COLOR
      graphics_context_set_text_color(ctx, GColorWhite);
    #elif PBL_BW
      graphics_context_set_text_color(ctx, GColorBlack);
    #endif
    graphics_draw_text(ctx, "Restart" , fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD), GRect(0,deck_ypos-5,60,card_height), GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL); 
  }
  
  //Draw cards
  int i;
  int k;
  int y_space = 16;
  for(i = 0; i < 4; ++i){
    if(card_top[i] == -1){
      graphics_fill_rect(ctx, GRect(card_1_xpos + i*(card_width + card_space), card_1_ypos, card_width, card_height), 0, GCornerNone); 
    }
    else{
      int cnt = 0;
      int max_cards;
      #ifdef PBL_COLOR
        max_cards = 3;
      #elif PBL_BW
        max_cards = 2;
      #endif      
      for(k = ((card_top[i] > max_cards ) ? (card_top[i]-max_cards): 0); k <= card_top[i]; ++k){
        graphics_draw_bitmap_in_rect(ctx, card_image_from_index(table[i][k]), GRect(card_1_xpos + i*(card_width + card_space), card_1_ypos+cnt*(y_space), card_width, card_height));     
        cnt++;
      }      
    }
  }
  
  //Draw pointer
  switch(pointer){
    case DECK :
      graphics_draw_bitmap_in_rect(ctx, pointer_image, GRect(deck_xpos+10, deck_ypos-10, 12, 10));
      break;
    case CARD_1 :
      graphics_draw_bitmap_in_rect(ctx, pointer_image, GRect(card_1_xpos+10, card_1_ypos-10, 12, 10));
      break;
    case CARD_2 :
      graphics_draw_bitmap_in_rect(ctx, pointer_image, GRect(card_1_xpos + (card_width + card_space)+10, card_1_ypos-10, 12, 10));
      break;
    case CARD_3 :
      graphics_draw_bitmap_in_rect(ctx, pointer_image, GRect(card_1_xpos + 2*(card_width + card_space)+10, card_1_ypos-10, 12, 10));
      break;
    case CARD_4 :
      graphics_draw_bitmap_in_rect(ctx, pointer_image, GRect(card_1_xpos + 3*(card_width + card_space)+10, card_1_ypos-10, 12, 10));
      break;   
  }
  //Draw select marker
  if (selected != -1){
    graphics_draw_bitmap_in_rect(ctx, pointer_image, GRect(card_1_xpos + selected*(card_width + card_space)-2, card_1_ypos-10, 12, 10));
    graphics_draw_bitmap_in_rect(ctx, pointer_image, GRect(card_1_xpos + selected*(card_width + card_space)+10, card_1_ypos-10, 12, 10));
    graphics_draw_bitmap_in_rect(ctx, pointer_image, GRect(card_1_xpos + selected*(card_width + card_space)+22, card_1_ypos-10, 12, 10));
  }  
  
  //Draw score
  graphics_context_set_text_color(ctx, GColorBlack);
  int cards_left = 0;
  char cards_left_str[20];
  char high_score_str[20];
  cards_left = 4 + card_top[0]+card_top[1]+card_top[2]+card_top[3]+(52-top_card_index);
  snprintf(cards_left_str, 20, "Cards left: %i", cards_left);
  graphics_draw_text(ctx, cards_left_str, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), (GRect) {.origin = {0, 0}, .size={140,14}}, GTextOverflowModeWordWrap, GTextAlignmentRight, NULL);
  if (won_once){
    snprintf(high_score_str, 20, "Times won: %i", high_score);
  }
  else{
    snprintf(high_score_str, 20, "Best round: %i", high_score);
  }
  graphics_draw_text(ctx, high_score_str, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), (GRect) {.origin = {0, 20}, .size={140,14}}, GTextOverflowModeWordWrap, GTextAlignmentRight, NULL);  

  //Win message
  if (won_round){
    graphics_context_set_text_color(ctx, GColorBlack);
    graphics_draw_text(ctx, "Good job!", fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD), (GRect) {.origin = {0, 125}, .size={144,30}}, GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);  
  }

}

static int rnd(int max){
  return rand() % max;
}

static void shuffle_deck(){
        int i;
        int j;
        int k;

        //Generate deck       
        for (i = 0; i < 52; ++i) {
                deck[i] = i;
        }
        //Shuffle deck
        for (i = 51; i >= 1; --i) {
                j = rnd(i);
                k = deck[j];
                deck[j] = deck[i];
                deck[i] = k;
        }
}

//For debugging!!
static void print_deck(){
  int i;
  for (i = top_card_index; i < 52; ++i){
    APP_LOG(APP_LOG_LEVEL_DEBUG, " %d",deck[i]);
  }
}
//Debugging
void print_card(int card) {
  static char *suits = "SDHC";
  static char faces[13] = {'A','2','3','4','5','6','7','8','9','X','J','Q','K'};
  APP_LOG(APP_LOG_LEVEL_DEBUG, "%c = %c%c",card,  faces[card%13], suits[card/13]);
}

static void load_images(){
  spades_image[0] = gbitmap_create_with_resource(RESOURCE_ID_ACE_OF_SPADES);
  spades_image[1] = gbitmap_create_with_resource(RESOURCE_ID_2_OF_SPADES);
  spades_image[2] = gbitmap_create_with_resource(RESOURCE_ID_3_OF_SPADES);
  spades_image[3] = gbitmap_create_with_resource(RESOURCE_ID_4_OF_SPADES);
  spades_image[4] = gbitmap_create_with_resource(RESOURCE_ID_5_OF_SPADES);
  spades_image[5] = gbitmap_create_with_resource(RESOURCE_ID_6_OF_SPADES);
  spades_image[6] = gbitmap_create_with_resource(RESOURCE_ID_7_OF_SPADES);
  spades_image[7] = gbitmap_create_with_resource(RESOURCE_ID_8_OF_SPADES);
  spades_image[8] = gbitmap_create_with_resource(RESOURCE_ID_9_OF_SPADES);
  spades_image[9] = gbitmap_create_with_resource(RESOURCE_ID_10_OF_SPADES);
  spades_image[10] = gbitmap_create_with_resource(RESOURCE_ID_JACK_OF_SPADES);
  spades_image[11] = gbitmap_create_with_resource(RESOURCE_ID_QUEEN_OF_SPADES);
  spades_image[12] = gbitmap_create_with_resource(RESOURCE_ID_KING_OF_SPADES);
  
  diamonds_image[0] = gbitmap_create_with_resource(RESOURCE_ID_ACE_OF_DIAMONDS);
  diamonds_image[1] = gbitmap_create_with_resource(RESOURCE_ID_2_OF_DIAMONDS);
  diamonds_image[2] = gbitmap_create_with_resource(RESOURCE_ID_3_OF_DIAMONDS);
  diamonds_image[3] = gbitmap_create_with_resource(RESOURCE_ID_4_OF_DIAMONDS);
  diamonds_image[4] = gbitmap_create_with_resource(RESOURCE_ID_5_OF_DIAMONDS);
  diamonds_image[5] = gbitmap_create_with_resource(RESOURCE_ID_6_OF_DIAMONDS);
  diamonds_image[6] = gbitmap_create_with_resource(RESOURCE_ID_7_OF_DIAMONDS);
  diamonds_image[7] = gbitmap_create_with_resource(RESOURCE_ID_8_OF_DIAMONDS);
  diamonds_image[8] = gbitmap_create_with_resource(RESOURCE_ID_9_OF_DIAMONDS);
  diamonds_image[9] = gbitmap_create_with_resource(RESOURCE_ID_10_OF_DIAMONDS);
  diamonds_image[10] = gbitmap_create_with_resource(RESOURCE_ID_JACK_OF_DIAMONDS);
  diamonds_image[11] = gbitmap_create_with_resource(RESOURCE_ID_QUEEN_OF_DIAMONDS);
  diamonds_image[12] = gbitmap_create_with_resource(RESOURCE_ID_KING_OF_DIAMONDS);
  
  hearts_image[0] = gbitmap_create_with_resource(RESOURCE_ID_ACE_OF_HEARTS);
  hearts_image[1] = gbitmap_create_with_resource(RESOURCE_ID_2_OF_HEARTS);
  hearts_image[2] = gbitmap_create_with_resource(RESOURCE_ID_3_OF_HEARTS);
  hearts_image[3] = gbitmap_create_with_resource(RESOURCE_ID_4_OF_HEARTS);
  hearts_image[4] = gbitmap_create_with_resource(RESOURCE_ID_5_OF_HEARTS);
  hearts_image[5] = gbitmap_create_with_resource(RESOURCE_ID_6_OF_HEARTS);
  hearts_image[6] = gbitmap_create_with_resource(RESOURCE_ID_7_OF_HEARTS);
  hearts_image[7] = gbitmap_create_with_resource(RESOURCE_ID_8_OF_HEARTS);
  hearts_image[8] = gbitmap_create_with_resource(RESOURCE_ID_9_OF_HEARTS);
  hearts_image[9] = gbitmap_create_with_resource(RESOURCE_ID_10_OF_HEARTS);
  hearts_image[10] = gbitmap_create_with_resource(RESOURCE_ID_JACK_OF_HEARTS);
  hearts_image[11] = gbitmap_create_with_resource(RESOURCE_ID_QUEEN_OF_HEARTS);
  hearts_image[12] = gbitmap_create_with_resource(RESOURCE_ID_KING_OF_HEARTS);
  
  clubs_image[0] = gbitmap_create_with_resource(RESOURCE_ID_ACE_OF_CLUBS);
  clubs_image[1] = gbitmap_create_with_resource(RESOURCE_ID_2_OF_CLUBS);
  clubs_image[2] = gbitmap_create_with_resource(RESOURCE_ID_3_OF_CLUBS);
  clubs_image[3] = gbitmap_create_with_resource(RESOURCE_ID_4_OF_CLUBS);
  clubs_image[4] = gbitmap_create_with_resource(RESOURCE_ID_5_OF_CLUBS);
  clubs_image[5] = gbitmap_create_with_resource(RESOURCE_ID_6_OF_CLUBS);
  clubs_image[6] = gbitmap_create_with_resource(RESOURCE_ID_7_OF_CLUBS);
  clubs_image[7] = gbitmap_create_with_resource(RESOURCE_ID_8_OF_CLUBS);
  clubs_image[8] = gbitmap_create_with_resource(RESOURCE_ID_9_OF_CLUBS);
  clubs_image[9] = gbitmap_create_with_resource(RESOURCE_ID_10_OF_CLUBS);
  clubs_image[10] = gbitmap_create_with_resource(RESOURCE_ID_JACK_OF_CLUBS);
  clubs_image[11] = gbitmap_create_with_resource(RESOURCE_ID_QUEEN_OF_CLUBS);
  clubs_image[12] = gbitmap_create_with_resource(RESOURCE_ID_KING_OF_CLUBS);
  
  card_back_image = gbitmap_create_with_resource(RESOURCE_ID_CARD_BACK);
  pointer_image = gbitmap_create_with_resource(RESOURCE_ID_POINTER);  
}

static void clear_table(){
  int i;
  int j;
  for(i = 0; i < 4; ++i){
    for(j = 0; j < 13; ++j){
      table[i][j] = -1;
    }
  }
}

static void deal(){  
  card_top[0]++;
  card_top[1]++;
  card_top[2]++;
  card_top[3]++;
  table[0][card_top[0]] = deck[top_card_index];
  table[1][card_top[1]] = deck[top_card_index+1];
  table[2][card_top[2]] = deck[top_card_index+2];
  table[3][card_top[3]] = deck[top_card_index+3];

  top_card_index += 4;

}

static void restart(){
  card_top[0] = -1;
  card_top[1] = -1;
  card_top[2] = -1;
  card_top[3] = -1;
  top_card_index = 0;
  selected = -1;
  won_round = false;
  
  clear_table();
  shuffle_deck();
  deal();  
}

static void load_score(){
  high_score = 52;
  won_once = false;

  if (persist_exists(HIGH_SCORE_KEY)) {
   high_score = persist_read_int(HIGH_SCORE_KEY);
  }
  if (persist_exists(WON_ONCE_KEY)){
    won_once = persist_read_int(WON_ONCE_KEY);
  }
}

static void save_score(){
  persist_write_int(HIGH_SCORE_KEY, high_score);
  persist_write_int(WON_ONCE_KEY, won_once);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  
  layer = layer_create(bounds);
  layer_set_update_proc(layer, layer_update_callback);
  layer_add_child(window_layer, layer);
    
  //Load images
  load_images();
  
  //Update times won    
  load_score();
  
  card_top[0] = -1;
  card_top[1] = -1;
  card_top[2] = -1;
  card_top[3] = -1;
  
  selected = -1;

  clear_table();
  shuffle_deck();
  deal();
   
  /*USED FOR TESTING
  top_card_index = 52;
  card_top[0] = 1;
  card_top[1] = 0;
  card_top[2] = 0;
  card_top[3] = 0;
  table[0][0] = 13;
  table[0][1] = 0;
  table[1][0] = 26;
  table[2][0] = 39;
  table[3][0] = 5;
  */
}

static void window_unload(Window *window) {
  for (int i = 0; i < 13; ++i) {
    gbitmap_destroy(spades_image[i]);
    gbitmap_destroy(clubs_image[i]);
    gbitmap_destroy(diamonds_image[i]);
    gbitmap_destroy(hearts_image[i]);
  }
  gbitmap_destroy(card_back_image);
  gbitmap_destroy(pointer_image);

  save_score();
  text_layer_destroy(text_layer);
}

static void init(void) {
  srand(time(NULL));
   
  window = window_create();
  window_set_click_config_provider(window, click_config_provider);
  window_set_window_handlers(window, (WindowHandlers) {
	.load = window_load,
    .unload = window_unload,
  });
  const bool animated = true;
  window_stack_push(window, animated);
}

static void deinit(void) {
  window_destroy(window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}