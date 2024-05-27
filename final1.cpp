#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <conio.h>
#include <windows.h>
#include <process.h>
#include <ctype.h>
#include <unistd.h>
#include <time.h>   // Required for Sleep function

#define YELLOW_COLOR "\033[1;33m"
#define RESET_COLOR "\033[0m"
#define CYAN_COLOR "\033[1;36m"
#define RED_COLOR "\033[1;31m"
#define BLUE_COLOR "\033[1;34m"
#define GREEN_COLOR "\033[1;32m"
#define MAGENTA_COLOR "\033[1;35m"
#define MAX_CHARACTERS 5
#define MAX_NAME_LENGTH 20
#define MAX_LINE_LENGTH 50 // For usleep


#define PACMAN 'C'
#define GHOST '@'
#define WIDTH 60
#define HEIGHT 15
#define EMPTY ' '
#define PELLET '.'
#define WALL '^'
#define NUM_STORIES 30

// Function prototypes
void chooseCharacter();
void displayCharacters() ;
int getUserCharacterChoice();
void showCharacterDescription(int character);
void printWithColor(const char* text, const char* color);
void printInBox(const char* text, const char* color, int boxWidth);
int getUserCharacterChoice2();

typedef struct {
    int x, y;
} Point;

typedef struct {
    Point pos;
    int dist;
} Node;

int pacman_x, pacman_y, ghost_x, ghost_y;
char grid[HEIGHT][WIDTH];
int distance[HEIGHT][WIDTH];
Point previous[HEIGHT][WIDTH];
int visited[HEIGHT][WIDTH];
int game_started = 0; // Flag to indicate whether the game has started or not
int game_over = 0;    // Flag to indicate whether the game is over or not
int pellets_eaten = 0; // Counter for pellets eaten by Pac-Man

void trim(char *str) {
    char *end;

    while (isspace((unsigned char)*str)) {
        str++;
    }

    if (*str == 0) {
        return;
    }

    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) {
        end--;
    }

    *(end + 1) = '\0';
}

int authenticate(char *username, char *password) {
    FILE *fp = fopen("login_file.csv", "r");

    if (fp == NULL) {
        printf(RED_COLOR"Error opening file!\n"RESET_COLOR);
        return 0;
    }

    char line[100];
    int authenticated = 0;
    int firstLine = 1;

    while (fgets(line, sizeof(line), fp) != NULL) {
        if (firstLine) {
            firstLine = 0;
            continue;
        }

        char *tok = strtok(line, ",");
        trim(tok);
        if (strcmp(tok, username) == 0) {
            tok = strtok(NULL, ",");
            trim(tok);
            if (strcmp(tok, password) == 0) {
                authenticated = 1;
                break;
            }
        }
    }

    fclose(fp);
    return authenticated;
}

void register_user(char *username, char *password) {
    FILE *fp = fopen("login_file.csv", "a");

    if (fp == NULL) {
        printf(RED_COLOR"Error opening file!\n"RESET_COLOR);
        return;
    }

    fprintf(fp, "%s,%s\n", username, password);

    fclose(fp);
}

void score_update(char *username, int score) {
    FILE *fp = fopen("score.csv", "r");
    FILE *temp_fp = fopen("temp_score.csv", "w");

    if (fp == NULL || temp_fp == NULL) {
        printf(RED_COLOR"Error opening file!\n"RESET_COLOR);
        if (fp != NULL) fclose(fp);
        if (temp_fp != NULL) fclose(temp_fp);
        return;
    }

    char line[100];
    int found = 0;

    trim(username);

    while (fgets(line, sizeof(line), fp) != NULL) {
        // Skip empty lines
        if (strlen(line) <= 1) // Check if the line contains only a newline character
            continue;

        char current_username[50];
        int current_score;
        sscanf(line, "%[^,],%d", current_username, &current_score);

        if (strcmp(current_username, username) == 0) {
            current_score += score;
            found = 1;
        }
        fprintf(temp_fp, "%s,%d\n", current_username, current_score);
    }

    // If the user wasn't found, add a new entry
    if (!found) {
        fprintf(temp_fp, "%s,%d\n", username, score);
    }

    fclose(fp);
    fclose(temp_fp);

    remove("score.csv");
    rename("temp_score.csv", "score.csv");
}

void sort_scores() {
    FILE *fp = fopen("score.csv", "r");
    FILE *temp_fp = fopen("temp_score.csv", "w");

    if (fp == NULL || temp_fp == NULL) {
        printf(RED_COLOR"Error opening file!\n"RESET_COLOR);
        if (fp != NULL) fclose(fp);
        if (temp_fp != NULL) fclose(temp_fp);
        return;
    }

    char lines[100][100];
    int numLines = 0;

    // Read all lines into an array
    char line[100];
    while (fgets(line, sizeof(line), fp) != NULL) {
        strcpy(lines[numLines], line);
        numLines++;
    }

    // Bubble sort the lines based on score
    for (int i = 0; i < numLines - 1; i++) {
        for (int j = 0; j < numLines - i - 1; j++) {
            int score1, score2;
            sscanf(lines[j], "%*[^,],%d", &score1);
            sscanf(lines[j + 1], "%*[^,],%d", &score2);
            if (score1 < score2) {
                char temp[100];
                strcpy(temp, lines[j]);
                strcpy(lines[j], lines[j + 1]);
                strcpy(lines[j + 1], temp);
            }
        }
    }

    // Write sorted lines to the temp file
    for (int i = 0; i < numLines; i++) {
        fputs(lines[i], temp_fp);
    }

    fclose(fp);
    fclose(temp_fp);

    remove("score.csv");
    rename("temp_score.csv", "score.csv");
}

// Function to set text color using ANSI escape codes
void setColor(const char* color) {
    printf("%s", color);
}

// Function to reset text color
void resetColor() {
    printf("\033[0m");
}

// Function to print the story
void printStory(int storyNum) {
    const char* color = "\033[1;36m"; // Cyan color for the story text
    setColor(color);

    switch (storyNum) {
        case 1:
            printf("Pacman's Patrol: As dawn broke, Pacman embarked on his daily patrol, collecting pellets scattered around the village. He greeted each villager with a cheerful wave. \"Another day, another pellet!\" he chirped, feeling a sense of duty and pride. His heart warmed seeing the village thrive. He knew every piece he gathered kept the village alive and safe. Today, he hoped for a ghost-free patrol.\n");
            break;
        case 2:
            printf("Drogon's Delight: Drogon roared a mighty roar as he chased the ghosts away from the village's borders. His powerful wings beat the air, creating gusts that sent the ghosts fleeing. Later, he found a field of flowers and rolled in them with a giggle. \"Flowers and powerpellets, my kind of day!\" he sighed happily. The villagers watched, amused by his playful nature. To them, he was a fierce protector with a heart of gold.\n");
            break;
        case 3:
            printf("Oggy's Adventure: Oggy climbed a tall tree to rescue a stuck kitten. Halfway up, he forgot how to get down and started to meow loudly for help. Pacman soon arrived and chuckled at the sight. \"You really are the cutest troublemaker,\" he said, gently lifting Oggy and the kitten down. Oggy purred gratefully, nuzzling Pacman's cheek. Together, they returned the kitten to its worried owner.\n");
            break;
        case 4:
            printf("Vendy's Vigilance: Vendy transformed from a vegetable into a vigilant warrior, his leafy limbs extending like powerful vines. He stood tall against the encroaching ghosts, his eyes fierce. \"Sir, this is Vendy's!\" he shouted, swinging his arms to fend them off. The ghosts hesitated, confused by his unusual form. With a swift move, he sent them retreating. The villagers cheered, grateful for his bravery.\n");
            break;
        case 5:
            printf("Ghostly Guardian: The Ghost prowled the forest cave, her senses always on high alert. She had once haunted this land but now stood as its greatest defender. When the village was in dire danger, she emerged with a mighty howl. Her presence alone struck fear into the hearts of hostile ghosts. \"I will protect this place,\" she vowed, her eyes glowing with determination. Tonight, she sensed a threat and was ready to act.\n");
            break;
        case 6:
            printf("Rebel Rumblings: In the graveyard, a small group of rebel ghosts whispered among themselves. \"One day, we'll join The Ghost and help the village,\" one said, longing for redemption. They remembered their lives before becoming ghosts. \"We owe it to our loved ones,\" another added, looking toward the village with hope. They knew the path to redemption wouldn't be easy. But united, they believed change was possible.\n");
            break;
        case 7:
            printf("Pacman and the Pellets: Pacman darted through the village, dodging a particularly persistent ghost. He spotted a pellet up ahead and sprinted towards it. \"Catch me if you can!\" he laughed, snatching the pellet just in time. The ghost groaned in frustration as Pacman disappeared around a corner. Returning to the village center, he deposited the pellet into the energy reservoir. Every pellet counted in their fight for survival.\n");
            break;
        case 8:
            printf("Drogon's Cuddle: After a long day of guarding the village, Drogon sought solace in The Ghost's cave. He curled up beside her, feeling the warmth of her presence. \"You always make me feel safe,\" he murmured, his eyes closing as exhaustion took over. The Ghost nuzzled him gently, her fierce exterior melting in his presence. Together, they drifted into a peaceful sleep. Tonight, they both found comfort in their bond.\n");
            break;
        case 9:
            printf("Oggy's Curiosity: Oggy found Vendy lounging in the meadow, basking in the sunlight. With a mischievous glint in his eye, Oggy pounced and batted at Vendy's leafy limbs. \"Playtime or nap time?\" he purred, his tail twitching with excitement. Vendy sighed in exasperation but couldn't hide his fondness for the little cat. \"Nap time, Oggy,\" he said, gently pushing him away. Reluctantly, Oggy settled down beside him, purring contentedly.\n");
            break;
        case 10:
            printf("Vendy's Catchphrase: As a ghost approached the village outskirts, Vendy stood his ground. His leafy arms rustled as he prepared for battle. \"Sir, this is Vendy's!\" he declared, his voice echoing with authority. The ghost paused, momentarily confused by the unusual warrior. Vendy took advantage of the hesitation and launched a swift attack. The ghost retreated, and Vendy smirked. \"Another day, another ghost,\" he muttered.\n");
            break;
        case 11:
            printf("Ghost's Reflection: The Ghost stood at the mouth of her cave, looking out over the peaceful village. Memories of her past life and the fear she once spread flooded her mind. \"I used to haunt this place... now I protect it,\" she mused, feeling a deep sense of redemption. She watched as Pacman and Drogon played in the distance, their laughter filling the air. \"I've come a long way,\" she thought, her resolve strengthening. She would protect this village at all costs.\n");
            break;
        case 12:
            printf("Rebel Leader's Secret: The rebel ghost leader floated near the village, her eyes fixed on Vendy. \"One day, I'll tell him how I feel,\" she thought, her ghostly heart fluttering with unspoken emotions. She admired his bravery and dedication to the village. \"But for now, I must remain a shadow,\" she sighed, watching from afar. She knew the risks of revealing herself too soon. Yet, the hope of a future together kept her going.\n");
            break;
        case 13:
            printf("Pacman's Bravery: A swarm of ghosts descended on the village, their eerie wails filling the air. Pacman stood firm, his heart pounding with determination. \"For the village!\" he shouted, charging at the ghosts with all his might. His swift movements and unwavering courage turned the tide of the battle. One by one, the ghosts retreated, unable to withstand his resolve. Exhausted but victorious, Pacman smiled, knowing he had protected his home once again.\n");
            break;
        case 14:
            printf("Drogon's Playtime: Drogon chased butterflies in the meadow, his fierce facade replaced by childlike joy. His massive wings created gentle breezes that lifted the butterflies higher. \"Even dragons need a break,\" he chuckled, catching a delicate butterfly on his nose. Villagers passing by watched in amusement, charmed by his playful antics. For a moment, the weight of his responsibilities lifted. Today, he was just a dragon enjoying the simple pleasures of life.\n");
            break;
        case 15:
            printf("Oggy's Meow: Oggy meowed loudly, summoning Pacman with an urgent tone. \"What's wrong, Oggy?\" Pacman asked, scanning the area for danger. Oggy pointed to a fallen bird, its tiny body trembling with fear. Together, they carefully picked it up and returned it to its nest. \"You're a hero, Oggy,\" Pacman said, patting the cat's head. Oggy purred, feeling proud of his good deed. The village thrived on small acts of kindness like this.\n");
            break;
        case 16:
            printf("Vendy's Frustration: Vendy found Oggy using him as a scratching post, his leafy limbs swaying with each playful swipe. \"Sir, this is Vendy's!\" he groaned, trying to remain patient. Oggy blinked up at him innocently, completely unphased. \"You're impossible,\" Vendy sighed, gently pushing the cat away. Despite his exasperation, he couldn't help but smile. Oggy's antics added a touch of lightheartedness to their daily lives.\n");
            break;
        case 17:
            printf("Ghost's Vigil: As night fell, The Ghost patrolled the village perimeter, her senses sharp and alert. The darkness was her ally, concealing her movements from the prying eyes of malevolent spirits. \"No ghost will harm my home,\" she vowed, her eyes glowing with an ethereal light. She moved silently, her presence a silent warning to any who dared to threaten the village. The villagers slept peacefully, unaware of the guardian watching over them. Tonight, the village was safe under her watchful gaze.\n");
            break;
        case 18:
            printf("Rebel's Hope: A rebel ghost placed a pellet near the village border, its translucent form flickering with nervous energy. \"For the villagers,\" it whispered, hoping to aid the living. It glanced around, ensuring no other ghosts saw its act of defiance. \"Maybe we can change,\" it thought, inspired by The Ghost's transformation. The ghost floated away, hoping its small gesture would make a difference. Redemption seemed closer than ever.\n");
            break;
        case 19:
            printf("Pacman's Determination: Pacman faced a ghost blocking his path, its menacing presence daunting. \"I won't let you harm them!\" he yelled, his voice steady with determination. He charged, weaving past the ghost with agility and speed. With a swift move, he grabbed the pellet and dashed back to safety. \"The village needs me,\" he thought, finding strength in his purpose. Every victory, no matter how small, was a step towards a safer home.\n");
            break;
        case 20:
            printf("Drogon's Snack: Drogon stumbled upon a hidden stash of powerpellets, his eyes lighting up with delight. \"Time for a snack!\" he grinned, chomping down on the pellets eagerly. Each bite filled him with a surge of power, his scales shimmering with renewed energy. \"These are definitely my ancestors' best invention,\" he chuckled, feeling invincible. He let out a mighty roar, ready to face any challenge. Today, no ghost stood a chance against him.\n");
            break;
        case 21:
            printf("Oggy's Mischief: Oggy found Pacman's helmet and wore it proudly, strutting around the village. \"Look at me, I'm Pacman!\" he meowed, his tail swishing with excitement. The villagers laughed, amused by Oggy's antics. Pacman soon spotted him and chuckled. \"Nice helmet, Oggy, but I think it suits me better,\" he said, gently retrieving his helmet. Oggy purred, happy to bring smiles to everyone's faces.\n");
            break;
        case 22:
            printf("Vendy's Heroics: Vendy stood between a ghost and the village children, his leafy limbs extending protectively. \"Not today, ghostie!\" he declared, swinging his arms with precision. The ghost recoiled, surprised by Vendy's fierce defense. The children cheered, feeling safe under Vendy's watchful eye. \"Sir, this is Vendy's!\" he added with a wink, making the children laugh. Vendy's heart swelled with pride, knowing he had protected the innocent.\n");
            break;
        case 23:
            printf("Ghost's Lament: The Ghost sighed as she stood at the edge of the village, remembering her past as a malevolent spirit. \"I was one of them once... but I've changed,\" she said softly, her gaze distant. She watched as the villagers went about their day, feeling a sense of belonging. \"I've come a long way,\" she thought, steeling herself for the next battle. Protecting the village had become her purpose. She would fight to keep it safe, no matter the cost.\n");
            break;
        case 24:
            printf("Rebel Leader's Secret: The rebel ghost leader floated near the village, her eyes fixed on Vendy as he trained. \"One day, I'll tell him how I feel,\" she thought, her ghostly heart fluttering with unspoken emotions. She admired his bravery and dedication to the village, feeling a deep connection. \"But for now, I must remain a shadow,\" she sighed, watching from afar. She knew the risks of revealing herself too soon. Yet, the hope of a future together kept her going.\n");
            break;
        case 25:
            printf("Pacman's Bravery: A swarm of ghosts descended on the village, their eerie wails filling the air. Pacman stood firm, his heart pounding with determination. \"For the village!\" he shouted, charging at the ghosts with all his might. His swift movements and unwavering courage turned the tide of the battle. One by one, the ghosts retreated, unable to withstand his resolve. Exhausted but victorious, Pacman smiled, knowing he had protected his home once again.\n");
            break;
        case 26:
            printf("Drogon's Roar: Drogon let out a roar that shook the trees, his powerful wings beating the air. The ghosts scattered, terrified of his immense power. \"Stay away from my home!\" he bellowed, his eyes fierce with determination. The villagers watched in awe, grateful for their formidable protector. Drogon landed gracefully, his fierce demeanor softening. He winked at the children who cheered him on, knowing he had done his job well.\n");
            break;
        case 27:
            printf("Oggy's Nap: Oggy curled up on Vendy's lap, purring contentedly as he dozed off. \"You're my favorite toy,\" he murmured sleepily, making Vendy chuckle. Despite his leafy exterior, Vendy gently patted Oggy's head, feeling a deep bond with the little cat. \"Rest well, Oggy,\" he whispered, enjoying the rare moment of peace. The sun set, casting a warm glow over the village. In this quiet moment, they both felt at home.\n");
            break;
        case 28:
            printf("Vendy's Resolve: Vendy faced the ghosts with unwavering determination, his leafy limbs poised for battle. \"For the village, and for love,\" he thought, inspired by feelings he didn't fully understand. His fierce defense sent the ghosts retreating, their attacks thwarted by his bravery. The villagers cheered, their faith in Vendy unwavering. \"Sir, this is Vendy's!\" he declared triumphantly, feeling a deep sense of accomplishment. Today, he fought not just for the village, but for the hope of a brighter future.\n");
            break;
        case 29:
            printf("Ghost's Protection: The Ghost leaped into action as ghosts attacked the village, her powerful form cutting through the shadows. \"You'll never harm them while I'm here,\" she growled, her eyes glowing with fierce determination. Her swift, decisive strikes sent the ghosts fleeing in terror. The villagers watched in awe, knowing they were safe under her protection. She stood tall, her presence a beacon of hope. \"This is my home now,\" she thought, ready to defend it with all her might.\n");
            break;
        case 30:
            printf("Rebel's Redemption: A rebel ghost saved a villager from an attack, its translucent form shimmering with determination. \"Maybe we can change,\" it said, looking towards The Ghost for guidance. The villager, initially frightened, realized the ghost's intention and nodded gratefully. \"Thank you,\" they whispered, recognizing the ghost's courage. The rebel floated back to the graveyard, feeling a glimmer of hope. Redemption seemed within reach, inspired by The Ghost's example.\n");
            break;
        default:
            printf("Invalid story number.\n");
            break;
    }

    resetColor();
}

// Function to initialize the grid
void render() {
    for (int i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {
            grid[i][j] = PELLET; // Default to placing pellets in all cells
        }
    }

    // Draw horizontal borders
    for (int j = 0; j < WIDTH; j++) {
        grid[0][j] = '-';
        grid[HEIGHT - 1][j] = '-';
    }

    // Draw vertical borders
    for (int i = 0; i < HEIGHT; i++) {
        grid[i][0] = '!';
        grid[i][WIDTH - 1] = '!';
    }

    // Place Pacman and Ghost
    pacman_x = WIDTH - 2; // Adjust to account for border
    pacman_y = HEIGHT - 2; // Adjust to account for border
    ghost_x = WIDTH / 2;
    ghost_y = HEIGHT / 2;
    grid[pacman_y][pacman_x] = PACMAN;
    grid[ghost_y][ghost_x] = GHOST;
}

void wall(int x, int y, int l, int b) {
    for (int i = 0; i < b; i++) {
        for (int j = 0; j < l; j++) {
            grid[HEIGHT - y - 1 - i][x + j] = WALL;
        }
    }
}

void movePacman(int move_x, int move_y) {
    int x = pacman_x + move_x;
    int y = pacman_y + move_y;

    if (grid[y][x] != '-' && grid[y][x] != '!' && grid[y][x] != WALL) {
        if (grid[y][x] == PELLET) {
            pellets_eaten++;
        }
        grid[pacman_y][pacman_x] = EMPTY;
        pacman_x = x;
        pacman_y = y;
        grid[pacman_y][pacman_x] = PACMAN;
    }
}

void moveGhost(Point next_step) {
    // Restore the previous position to a pellet if it was a pellet
    if (grid[ghost_y][ghost_x] == GHOST) {
        grid[ghost_y][ghost_x] = PELLET;
    } else {
        grid[ghost_y][ghost_x] = EMPTY;
    }

    ghost_x = next_step.x;
    ghost_y = next_step.y;
    // Place the ghost at the new position
    grid[ghost_y][ghost_x] = GHOST;
}

void draw() {
    // Move the cursor to the top-left corner of the screen
    printf("\033[H");

    for (int i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {
            if (grid[i][j] == WALL) {
                printf(CYAN_COLOR "%c" RESET_COLOR, grid[i][j]);
            } else if (grid[i][j] == GHOST) {
                printf(RED_COLOR "%c" RESET_COLOR, grid[i][j]);
            } else if (grid[i][j] == PACMAN) {
                printf(YELLOW_COLOR "%c" RESET_COLOR, grid[i][j]);
            } else if (grid[i][j] == PELLET) {
                printf(MAGENTA_COLOR "%c" RESET_COLOR, grid[i][j]);
            } else {
                printf("%c", grid[i][j]);
            }
        }
        printf("\n");
    }

    fflush(stdout); // Flush the standard output buffer
}

int is_valid(int x, int y) {
    return (x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT && grid[y][x] != WALL && grid[y][x] != '!' && grid[y][x] != '-');
}

void dijkstra(Point src, Point dest) {
    for (int i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {
            distance[i][j] = INT_MAX;
            previous[i][j] = (Point){-1, -1};
            visited[i][j] = 0;
        }
    }

    distance[src.y][src.x] = 0;
    Node nodes[HEIGHT * WIDTH];
    int node_count = 0;
    nodes[node_count++] = (Node){src, 0};

    int directions[4][2] = {{0, 1}, {1, 0}, {0, -1}, {-1, 0}};
    while (node_count > 0) {
        Node min_node = nodes[0];
        int min_index = 0;
        for (int i = 1; i < node_count; i++) {
            if (nodes[i].dist < min_node.dist) {
                min_node = nodes[i];
                min_index = i;
            }
        }
        nodes[min_index] = nodes[--node_count];
        int x = min_node.pos.x;
        int y = min_node.pos.y;
        if (visited[y][x]) continue;
        visited[y][x] = 1;
        for (int i = 0; i < 4; i++) {
            int nx = x + directions[i][0];
            int ny = y + directions[i][1];
            if (is_valid(nx, ny) && !visited[ny][nx]) {
                int new_dist = distance[y][x] + 1;
                if (new_dist < distance[ny][nx]) {
                    distance[ny][nx] = new_dist;
                    previous[ny][nx] = (Point){x, y};
                    nodes[node_count++] = (Node){(Point){nx, ny}, new_dist};
                }
            }
        }
    }
}

Point get_next_step(Point src, Point dest) {
    if (distance[dest.y][dest.x] == INT_MAX) {
        // No path found
        return src;
    }
    Point current = dest;
    while (previous[current.y][current.x].x != -1 && previous[current.y][current.x].y != -1) {
        if (previous[current.y][current.x].x == src.x && previous[current.y][current.x].y == src.y) {
            return current;
        }
        current = previous[current.y][current.x];
    }
    return src;
}

void moveGhostContinuously() {
    while (!game_over) {
        Point src = {ghost_x, ghost_y};
        Point dest = {pacman_x, pacman_y};
        dijkstra(src, dest);

        // Move the ghost two spaces at a time
        for (int i = 0; i < 2; i++) {
            Point next_step = get_next_step(src, dest);
            if (next_step.x == pacman_x && next_step.y == pacman_y) {
                game_over = 1; // Stop the game if ghost eats Pacman
                grid[pacman_y][pacman_x] = GHOST; // Move the ghost to Pacman's position
                return; // Exit the function without printing "Game Over!"
            }
            moveGhost(next_step);
            Sleep(1000); // Pause for 1 second between each move
            src = (Point){ghost_x, ghost_y}; // Update source after each move
        }

        // Clear the screen before redrawing
        system("cls");

        // Draw the grid after each move
        draw();
    }
}


#define BLUE "\033[1;34m"
#define RESET "\033[0m"

// Function to convert integer to Roman numeral
const char *int_to_roman(int num) {
    static char roman[20];
    memset(roman, 0, sizeof(roman));

    struct {
        int value;
        const char *symbol;
    } roman_map[] = {
        {1000, "M"}, {900, "CM"}, {500, "D"}, {400, "CD"},
        {100, "C"}, {90, "XC"}, {50, "L"}, {40, "XL"},
        {10, "X"}, {9, "IX"}, {5, "V"}, {4, "IV"}, {1, "I"}
    };

    for (int i = 0; num > 0 && i < sizeof(roman_map)/sizeof(roman_map[0]); i++) {
        while (num >= roman_map[i].value) {
            strcat(roman, roman_map[i].symbol);
            num -= roman_map[i].value;
        }
    }

    return roman;
}

void display_scoreboard() {
    FILE *fp = fopen("score.csv", "r");

    if (fp == NULL) {
        printf("Error opening file!\n");
        return;
    }

    char line[100];
    int position = 1;

    printf( 			"                           ======================================================\n" );
    printf(YELLOW_COLOR "                                          WELCOME TO THE SCOREBOARD\n" RESET_COLOR);
    printf( 			"                           ======================================================\n" );
    printf(BLUE_COLOR "| %-10s | %-20s | %-10s \n" RESET_COLOR, "POSITION", "USERNAME", "SCORE");
    printf( "---------------------------------------------\n" );

    while (fgets(line, sizeof(line), fp) != NULL) {
        char username[50];
        int score;
        sscanf(line, "%[^,],%d", username, &score);

        printf(BLUE_COLOR "| " YELLOW_COLOR "%-10s" BLUE_COLOR " | " RED_COLOR "%-20s" BLUE_COLOR " | " GREEN_COLOR "%-10d\n" RESET_COLOR, 
               int_to_roman(position), username, score);
        printf( "---------------------------------------------\n" );
        
        usleep(350000);
        
		position++;
    }

    fclose(fp);
}

void chooseCharacter() {
    int finalChosenCharacter = 0;
    char viewMoreDescriptions;

    displayCharacters();
    
    do {
        int chosenCharacter = getUserCharacterChoice();
        showCharacterDescription(chosenCharacter);

        printf("Do you want to see another character's description? (y/n): ");
        scanf(" %c", &viewMoreDescriptions); // The space before %c is to consume any leftover newline character

    } while (viewMoreDescriptions == 'y' || viewMoreDescriptions == 'Y');

    finalChosenCharacter = getUserCharacterChoice2();
    printf("You have chosen:\n");
    showCharacterDescription(finalChosenCharacter);
}


// Function to display character options with colors in a box
void displayCharacters() {
    int maxLineLength = 50; // Maximum line length including color codes
    char line[maxLineLength];
    FILE *file = fopen("character_file1.txt", "r");
    if (file == NULL) {
        printf("Error opening file.\n");
        exit(1);
    }

    // Read and discard the first line
    fgets(line, maxLineLength, file);

    // Initialize variables to store maximum lengths of each field
    int maxNameLength = 0, maxSpeedLength = 0, maxScoreLength = 0;

    // Find maximum lengths of each field
    while (fgets(line, maxLineLength, file) != NULL) {
        char name[MAX_NAME_LENGTH], character, speed[3], score_needed[5];
        sscanf(line, "%[^,],%c,%[^,],%s", name, &character, speed, score_needed);

        int nameLength = strlen(name);
        if (nameLength > maxNameLength) {
            maxNameLength = nameLength;
        }

        int speedLength = strlen(speed);
        if (speedLength > maxSpeedLength) {
            maxSpeedLength = speedLength;
        }

        int scoreLength = strlen(score_needed);
        if (scoreLength > maxScoreLength) {
            maxScoreLength = scoreLength;
        }
    }

    // Reset file pointer to the beginning of the file
    fseek(file, 0, SEEK_SET);

    // Read and discard the first line again
    fgets(line, maxLineLength, file);

    // Print the rest of the lines from the file with proper alignment
    while (fgets(line, maxLineLength, file) != NULL) {
        // Remove newline character if present
        char *newline = strchr(line, '\n');
        if (newline != NULL) {
            *newline = '\0';
        }

        // Parse character information from the line
        char name[MAX_NAME_LENGTH], character, speed[3], score_needed[5];
        sscanf(line, "%[^,],%c,%[^,],%s", name, &character, speed, score_needed);

        // Determine color based on character
        const char *color;
        switch (character) {
            case 'C':
                color = "\033[1;36m"; // Cyan
                break;
            case 'D':
                color = "\033[1;32m"; // Green
                break;
            case 'O':
                color = "\033[1;33m"; // Yellow
                break;
            case 'V':
                color = "\033[1;34m"; // Blue
                break;
            case 'Q':
                color = "\033[1;35m"; // Magenta
                break;
            default:
                color = "\033[1;31m"; // Red
                break;
        }

        // Print character information with ANSI color codes and proper alignment
        printf("%s| %-*s | %-*c | %-*s | %-*s |%s\n", color, maxNameLength, name, maxNameLength, character, maxSpeedLength, speed, maxScoreLength, score_needed, RESET_COLOR);
    }
    fclose(file);
}


// Function to get user input for character choice to view description
int getUserCharacterChoice() {
    int character;
    int boxWidth = 30; // Set a consistent width for all boxes
    printInBox("Enter the character number to VIEW DESCRIPTION (1-5) : ", CYAN_COLOR, boxWidth);
    scanf("%d", &character);
    
    while (character < 1 || character > 5) {
        printInBox("Invalid choice. Please enter a number between 1 and 5: ", RED_COLOR, boxWidth);
        scanf("%d", &character);
    }
    return character;
}
//Function to get user input for character choice to play
int getUserCharacterChoice2() {
    int character;
    int boxWidth = 30; // Set a consistent width for all boxes
    printInBox("Enter the character number to PLAY (1-5) : ", CYAN_COLOR, boxWidth);
    scanf("%d", &character);
    
    while (character < 1 || character > 5) {
        printInBox("Invalid choice. Please enter a number between 1 and 5: ", RED_COLOR, boxWidth);
        scanf("%d", &character);
    }
    return character;
}


// Function to show description for the chosen character
void showCharacterDescription(int character) {
    switch(character) {
        case 1:
            printInBox("PACMAN: The classic hero of our game, Pacman is quick and agile, with a speed of 2. Starting with a score of 0, Pacman is ready to munch through the maze!", GREEN_COLOR, 70);
            break;
        case 2:
            printInBox("DROGON: With a speed of 4, Drogon the dragon soars through the maze. Starting with a score of 100, he’s ready to take on any challenge!", YELLOW_COLOR, 70);
            break;
        case 3:
            printInBox("OGGY: The speedy cat, Oggy, races through the maze with a speed of 6. Starting with a score of 200, Oggy is always on the hunt for pellets!", BLUE_COLOR, 70);
            break;
        case 4:
            printInBox("VENDY: The agile Vendy moves at a speed of 8. Starting with a score of 400, Vendy navigates the maze with precision and skill!", MAGENTA_COLOR, 70);
            break;
        case 5:
            printInBox("THE GHOST: The most feared character with a speed of 9. Starting with a score of 800, The Ghost haunts the maze and sends chills down the spine of any opponent!", RED_COLOR, 70);
            break;
        default:
            printInBox("Invalid character selection.", RED_COLOR, 50);
            break;
    }
}

// Function to print text with a specific color
void printWithColor(const char* text, const char* color) {
    printf("%s%s%s", color, text, RESET_COLOR);
}

// Function to print text inside a box with a specific color
void printInBox(const char* text, const char* color, int boxWidth) {
    int len = strlen(text);
    int padding = (boxWidth - len - 2) / 2;
    int paddingExtra = (boxWidth - len - 2) % 2;

    // Print top border
    printf("%s+", color);
    for (int i = 0; i < boxWidth; ++i) {
        printf("-");
    }
    printf("+%s\n", RESET_COLOR);
    
    // Print text with side borders and padding
    printf("%s|", color);
    for (int i = 0; i < padding; ++i) {
        printf(" ");
    }
    printf(" %s", text);
    for (int i = 0; i < padding + paddingExtra; ++i) {
        printf(" ");
    }
    printf("|%s\n", RESET_COLOR);
    
    // Print bottom border
    printf("%s+", color);
    for (int i = 0; i < boxWidth; ++i) {
        printf("-");
    }
    printf("+%s\n", RESET_COLOR);
    
    usleep(300000); // Sleep for 300ms to simulate delay
}



int main() {
    char username[50], password[50];
    char choice;

    printf( RED_COLOR"                           ======================================================\n"RESET_COLOR );
    printf(YELLOW_COLOR "                                       WELCOME TO THE B2-6 PACMAN GAME           \n" RESET_COLOR);
    printf( RED_COLOR"                           ======================================================\n"RESET_COLOR );
    printf(CYAN_COLOR"\nAre you ready to embark on a thrilling adventure ? \n(Press 's' to sign in, 'u' to sign up): "RESET_COLOR);
    scanf(" %c", &choice);

    if (choice == 's') {
        printf(YELLOW_COLOR"Enter your username: "RESET_COLOR);
        scanf("%s", username);

        printf(RED_COLOR"Enter your password: "RESET_COLOR);
        scanf("%s", password);

        if (authenticate(username, password)) {
            printf(MAGENTA_COLOR"Authentication successful! Welcome to the maze adventure...\n"RESET_COLOR);
        } else {
            printf(RED_COLOR"Authentication failed! You need the right credentials to enter this maze.\n"RESET_COLOR);
            return 1;
        }
    } else if (choice == 'u') {
        printf(YELLOW_COLOR"Create a username: "RESET_COLOR);
        scanf("%s", username);

        printf(RED_COLOR"Create a password: "RESET_COLOR);
        scanf("%s", password);

        register_user(username, password);

        printf(MAGENTA_COLOR"Account created successfully! Welcome to the maze adventure...\n"RESET_COLOR);
    } else {
        printf(RED_COLOR"Oh no! That's not a valid choice. Let's try again later!\n"RESET_COLOR);
        return 1;
    }

	srand(time(NULL));
    int storyNum = (rand() % NUM_STORIES) + 1;

    printf(YELLOW_COLOR"Its story time!!!\n"RESET_COLOR);
    printf(YELLOW_COLOR"Here's a characters story for you while you wait:\n\n"RESET_COLOR);

    printStory(storyNum);
    printf(MAGENTA_COLOR"\n\n\n                                     LOADING"RESET_COLOR);
    sleep(2);
    printf(MAGENTA_COLOR"."RESET_COLOR);
    sleep(2);
    printf(MAGENTA_COLOR"."RESET_COLOR);
    sleep(2);
    printf(MAGENTA_COLOR"."RESET_COLOR);
    sleep(2);
    printf(MAGENTA_COLOR"."RESET_COLOR);
    sleep(2);
    printf(MAGENTA_COLOR"."RESET_COLOR);
    printf(YELLOW_COLOR"\n\n\nThank you for waiting! Please proceed to the game.\n"RESET_COLOR); 
	printf(GREEN_COLOR"| NAME | CHARACTER | SPEED | SCORE |\n"RESET_COLOR);
	printf(GREEN_COLOR"------------------------------------\n"RESET_COLOR);
    chooseCharacter();
    sleep(6);
	render();
    // Clear the screen before redrawing
        system("cls"); 
    // Predefined walls
    int walls[14][4] = {
        {1, 1, 4, 3},
        {1, 10, 5, 4},
        {10, 9, 5, 2},
        {10, 11, 2, 3},
        {26, 5, 2, 4},
        {28, 5, 5, 2},
        {33, 5, 2, 4},
        {40, 1, 3, 5},
        {43, 5, 5, 1},
        {46, 3, 2, 2},
        {11, 3, 8, 2},
        {50, 10, 7, 2},
        {47, 10, 3, 3}
    };

    for (int i = 0; i < 14; i++) {
        wall(walls[i][0], walls[i][1], walls[i][2], walls[i][3]);
    }

    Point src = {ghost_x, ghost_y};
    Point dest = {pacman_x, pacman_y};

    dijkstra(src, dest);
    draw();
    HANDLE thread_handle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)moveGhostContinuously, NULL, 0, NULL);

    printf( RED_COLOR"                              ======================================================\n"RESET_COLOR );
    printf(YELLOW_COLOR "                                              GET READY TO PLAY!                    \n" RESET_COLOR);
    printf( RED_COLOR"                              ======================================================\n"RESET_COLOR );
    printf(CYAN_COLOR"\nPress any key to enter the maze...\n"RESET_COLOR);
    getch(); // Wait for any key press
    
    char ch;
    while (!game_over) {
        draw();
        ch = getch();
        switch (ch) {
            case 'w':
                movePacman(0, -1); // Move up
                break;
            case 's':
                movePacman(0, 1); // Move down
                break;
            case 'a':
                movePacman(-1, 0); // Move left
                break;
            case 'd':
                movePacman(1, 0); // Move right
                break;
            case 'q':
                game_over = 1;
                break;
        }
        if (pacman_x == ghost_x && pacman_y == ghost_y) {
            game_over = 1; // Stop the game if ghost eats Pacman
            grid[pacman_y][pacman_x] = GHOST; // Move the ghost to Pacman's position
            draw(); // Redraw the grid to show the final state
            
        }
    }

    // Wait for the ghost continuous movement thread to finish
    WaitForSingleObject(thread_handle, INFINITE);

    // Print the game over message and the total number of pellets eaten
    printf(RED_COLOR "Game Over!\n" RESET_COLOR);
    printf(YELLOW_COLOR"Pellets eaten: %d\n"RESET_COLOR, pellets_eaten);
    
    int score = pellets_eaten * 10;
	printf(YELLOW_COLOR"the score is : %d\n"RESET_COLOR,score);
	
	usleep(350000);
	
    score_update(username, score);
    sort_scores();
	display_scoreboard();
    return 0;
	
}

