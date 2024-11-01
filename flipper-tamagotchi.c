#include <furi.h>
#include <gui/gui.h>
#include <input/input.h>
#include <nfc.h>
#include <storage/storage.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <stdio.h>

// Structure pour le Tamagotchi
typedef struct {
    int hunger;
    int happiness;
    int fatigue;
    int cleanliness;
    int age;
    int experience;
    int level;
    int health;
    char personality[20];
} Tamagotchi;

// Structure pour l'inventaire
typedef struct {
    int medicine_count;
    int treats;
    int toys;
} Inventory;

// Structure pour les quêtes quotidiennes
typedef struct {
    int feed_goal;
    int play_goal;
    int clean_goal;
    int feed_count;
    int play_count;
    int clean_count;
    bool completed;
} Quest;

// Initialisation des structures
static Tamagotchi pet = {100, 100, 100, 100, 0, 0, 1, 100, "joyeux"};
static Inventory inventory = {3, 5, 3};
static Quest daily_quest = {3, 2, 1, 0, 0, 0, false};

// Personnalités possibles pour le Tamagotchi
const char* personalities[] = {"joyeux", "gourmand", "paresseux", "energetique", "timide"};

// Fonction pour assigner une personnalité aléatoire
void assign_random_personality(Tamagotchi* pet) {
    srand(time(NULL));
    int random_index = rand() % (sizeof(personalities) / sizeof(personalities[0]));
    snprintf(pet->personality, sizeof(pet->personality), "%s", personalities[random_index]);
}

// Fonction de sauvegarde et de chargement
void save_game(Tamagotchi* pet, Inventory* inventory, Quest* quest) {
    Storage* storage = furi_record_open("storage");
    storage_write(storage, "/ext/tamagotchi_data", pet, sizeof(Tamagotchi));
    storage_write(storage, "/ext/inventory_data", inventory, sizeof(Inventory));
    storage_write(storage, "/ext/quest_data", quest, sizeof(Quest));
    furi_record_close("storage");
}

void load_game(Tamagotchi* pet, Inventory* inventory, Quest* quest) {
    Storage* storage = furi_record_open("storage");
    if (storage_file_exists(storage, "/ext/tamagotchi_data")) {
        storage_read(storage, "/ext/tamagotchi_data", pet, sizeof(Tamagotchi));
        storage_read(storage, "/ext/inventory_data", inventory, sizeof(Inventory));
        storage_read(storage, "/ext/quest_data", quest, sizeof(Quest));
    }
    furi_record_close("storage");
}

// Mise à jour de l'état du Tamagotchi
void update_tamagotchi(Tamagotchi* pet, int hunger_change, int happiness_change, int fatigue_change, int cleanliness_change) {
    // Ajustements en fonction de la personnalité
    if (strcmp(pet->personality, "gourmand") == 0) hunger_change -= 5;
    if (strcmp(pet->personality, "joyeux") == 0) happiness_change += 2;
    if (strcmp(pet->personality, "paresseux") == 0) fatigue_change += 5;
    if (strcmp(pet->personality, "energetique") == 0) fatigue_change -= 3;
    if (strcmp(pet->personality, "timide") == 0) happiness_change -= 3;

    pet->hunger += hunger_change;
    pet->happiness += happiness_change;
    pet->fatigue += fatigue_change;
    pet->cleanliness += cleanliness_change;

    // Limiter les valeurs entre 0 et 100
    if (pet->hunger < 0) pet->hunger = 0;
    if (pet->hunger > 100) pet->hunger = 100;
    if (pet->happiness < 0) pet->happiness = 0;
    if (pet->happiness > 100) pet->happiness = 100;
    if (pet->fatigue < 0) pet->fatigue = 0;
    if (pet->fatigue > 100) pet->fatigue = 100;
    if (pet->cleanliness < 0) pet->cleanliness = 0;
    if (pet->cleanliness > 100) pet->cleanliness = 100;
}

// Affichage de l'état actuel du Tamagotchi
void draw_callback(Canvas* canvas, void* ctx) {
    canvas_clear(canvas);
    canvas_draw_str(canvas, 10, 10, "Tamagotchi");
    canvas_draw_str(canvas, 10, 30, "Faim:"); canvas_draw_int(canvas, 60, 30, pet.hunger);
    canvas_draw_str(canvas, 10, 50, "Bonheur:"); canvas_draw_int(canvas, 60, 50, pet.happiness);
    canvas_draw_str(canvas, 10, 70, "Fatigue:"); canvas_draw_int(canvas, 60, 70, pet.fatigue);
    canvas_draw_str(canvas, 10, 90, "Proprete:"); canvas_draw_int(canvas, 60, 90, pet.cleanliness);
    canvas_draw_str(canvas, 10, 110, "Age:"); canvas_draw_int(canvas, 60, 110, pet.age);
    canvas_draw_str(canvas, 10, 130, "Niveau:"); canvas_draw_int(canvas, 60, 130, pet.level);
    canvas_draw_str(canvas, 10, 150, "Santé:"); canvas_draw_int(canvas, 60, 150, pet.health);
}

// Utiliser un médicament pour soigner le Tamagotchi
void use_medicine(Tamagotchi* pet, Inventory* inventory) {
    if (inventory->medicine_count > 0 && pet->health < 100) {
        inventory->medicine_count -= 1;
        pet->health += 20;
        if (pet->health > 100) pet->health = 100;
        printf("Un médicament a été utilisé. Santé restaurée de 20 points.\n");
    }
}

// Utiliser une friandise pour augmenter le bonheur
void use_treat(Tamagotchi* pet, Inventory* inventory) {
    if (inventory->treats > 0) {
        inventory->treats -= 1;
        pet->happiness += 15;
        if (pet->happiness > 100) pet->happiness = 100;
        printf("Une friandise a été utilisée. Bonheur augmenté de 15 points.\n");
    }
}

// Utiliser un jouet pour réduire la fatigue
void use_toy(Tamagotchi* pet, Inventory* inventory) {
    if (inventory->toys > 0) {
        inventory->toys -= 1;
        pet->fatigue -= 15;
        if (pet->fatigue < 0) pet->fatigue = 0;
        printf("Un jouet a été utilisé. Fatigue réduite de 15 points.\n");
    }
}

// Vérification des quêtes quotidiennes
void check_quest_progress(Quest* quest, Inventory* inventory) {
    if (quest->feed_count >= quest->feed_goal &&
        quest->play_count >= quest->play_goal &&
        quest->clean_count >= quest->clean_goal &&
        !quest->completed) {
        quest->completed = true;
        inventory->treats += 1;
        inventory->toys += 1;
        printf("Quête quotidienne terminée ! Vous recevez une friandise et un jouet !\n");
    }
}

// Évolution du Tamagotchi
void check_evolution(Tamagotchi* pet) {
    if (pet->age >= 10 && pet->level >= 5) {
        if (strcmp(pet->personality, "joyeux") == 0) {
            pet->happiness += 20;
            if (pet->happiness > 100) pet->happiness = 100;
            printf("Votre Tamagotchi a évolué en une créature radieuse !\n");
        } else if (strcmp(pet->personality, "energetique") == 0) {
            pet->fatigue -= 20;
            if (pet->fatigue < 0) pet->fatigue = 0;
            printf("Votre Tamagotchi a évolué en une créature rapide !\n");
        }
        pet->age = 0;
        pet->level += 1;
    }
}

// Gestion des entrées utilisateur
void input_callback(InputEvent* input_event, void* ctx) {
    if(input_event->type == InputTypePress) {
        if(input_event->key == InputKeyUp) {
            update_tamagotchi(&pet, -10, 5, 0, -5);
            daily_quest.feed_count++;
        } else if(input_event->key == InputKeyDown) {
            update_tamagotchi(&pet, 0, 10, -10, 0);
            daily_quest.play_count++;
        } else if(input_event->key == InputKeyLeft) {
            update_tamagotchi(&pet, 0, 0, 0, 10);
            daily_quest.clean_count++;
        } else if(input_event->key == InputKeyOk) {
            use_medicine(&pet, &inventory);
        } else if(input_event->key == InputKeyPlus) {
            use_treat(&pet, &inventory);
        } else if(input_event->key == InputKeyMinus) {
            use_toy(&pet, &inventory);
        }
        check_quest_progress(&daily_quest, &inventory);
        save_game(&pet, &inventory, &daily_quest);
    }
}

// Fonction principale
int main() {
    Gui* gui = furi_record_open("gui");
    ViewPort* viewport = view_port_alloc();
    view_port_draw_callback_set(viewport, draw_callback, NULL);
    view_port_input_callback_set(viewport, input_callback, NULL);
    gui_add_view_port(gui, viewport, GuiLayerFullscreen);

    load_game(&pet, &inventory, &daily_quest);
    assign_random_personality(&pet);

    int event_timer = 0;
    while (1) {
        check_evolution(&pet);
        check_quest_progress(&daily_quest, &inventory);
        if (event_timer >= 60) {
            event_timer = 0; // Placez ici les événements aléatoires si nécessaire
        }
        furi_delay_ms(5000);
        event_timer += 5;
    }

    gui_remove_view_port(gui, viewport);
    view_port_free(viewport);
    furi_record_close("gui");
    return 0;
}
