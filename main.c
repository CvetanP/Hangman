#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#define MAX_WORD_LENGTH 50
#define MAX_TRIES 6
#define MAX_WORDS 100

// Struct to hold a word and its hint
struct WordWithHint
{
    char word[MAX_WORD_LENGTH];
    char hint[MAX_WORD_LENGTH];
    char difficulty[10]; // easy, medium, or hard
};

// Function to display the current state of the word
void displayWord(const char word[], const bool guessed[]);

// Function to draw the hangman
void drawHangman(int tries);

// Function to load the words from the file
int loadWords(const char *filename, struct WordWithHint wordList[], int maxWords);

// Function to filter the words
int filterWordsByDifficulty(struct WordWithHint wordList[], int wordCount, struct WordWithHint filteredList[], const char *difficulty);

// Play Again lol
bool playAgain();

// Read the high score from the file
int readHighScore(const char *filename);

// Updated when the new score is beaten
void updateHighScore(const char *filename, int score);

// Calculate what score we are on(It was in the main function before but though of moving it here)
int calculateScore(const char *difficulty, bool wordGuessed);

// run code
int main()
{
    // Seed the random number generator
    srand(time(NULL));
    struct WordWithHint wordList[MAX_WORDS];
    int wordCount = loadWords("words.txt", wordList, MAX_WORDS);

    if (wordCount == 0)
    {
        printf("No words loaded. Exiting program.\n"); // a validation to make sure the file is there
        return 1;
    }

    // Display current high score
    int highScore = readHighScore("highscore.txt");
    printf("Current High Score: %d\n", highScore);

    // Initialize currentScore to 0 at the start of the game
    int currentScore = 0;

    // loop so we can replay the game
    do
    {
        // Select difficulty level
        char difficulty[10];
        printf("Choose difficulty (easy, medium, hard): ");
        scanf("%s", difficulty);

        struct WordWithHint filteredList[MAX_WORDS];
        int filteredCount = filterWordsByDifficulty(wordList, wordCount, filteredList, difficulty);

        if (filteredCount == 0)
        {
            printf("No words available for the selected difficulty. Exiting program.\n"); // a validation to make sure the file is there
            return 1;
        }

        // Select a random word from the filtered list
        int wordIndex = rand() % filteredCount;

        const char *secretWord = filteredList[wordIndex].word;
        const char *hint = filteredList[wordIndex].hint;

        int wordLength = strlen(secretWord);
        char guessedWord[MAX_WORD_LENGTH] = {0};

        for (int i = 0; i < wordLength; i++)
        {
            guessedWord[i] = '_'; // Fill with underscores
        }
        guessedWord[wordLength] = '\0'; // Null-terminate the string
        bool guessedLetters[26] = {false};

        printf("Welcome to Hangman!\n");
        printf("Hint: %s\n", hint);

        int tries = 0;
        bool wordGuessed = false; // it's needed down below for the high score
        while (tries < MAX_TRIES)
        {
            printf("\n");
            displayWord(guessedWord, guessedLetters);
            drawHangman(tries);

            char guess;
            printf("Enter a letter: ");
            scanf(" %c", &guess);
            guess = tolower(guess);

            // Validate if the input is a valid letter
            if (guess < 'a' || guess > 'z')
            {
                printf("Invalid input. Please enter a letter from 'a' to 'z'.\n");
                continue; // Skip the rest of the loop and ask for input again
            }

            if (guessedLetters[guess - 'a']) // Check if the letter has already been guessed
            {
                printf("You've already guessed that letter. Try again.\n");
                continue;
            }

            guessedLetters[guess - 'a'] = true;

            bool found = false;
            for (int i = 0; i < wordLength; i++) // $2 optimized for this cuz 'a' in ASCII is 97 so instead of 97-122 loops we get 1-25 loop which is faster.
            {
                if (secretWord[i] == guess)
                {
                    found = true;
                    guessedWord[i] = guess;
                }
            }

            if (found)
            {
                printf("Good guess!\n");
            }
            else
            {
                printf("Sorry, the letter '%c' is not in the word.\n", guess);
                tries++;
            }

            if (strcmp(secretWord, guessedWord) == 0)
            {
                printf("\nCongratulations! You've guessed the word: %s\n", secretWord);
                wordGuessed = true;
                break;
            }
        }

        if (!wordGuessed)
        {
            printf("\nSorry, you've run out of tries. The word was: %s\n", secretWord);
            break;
        }

        // Calculate score using the new function
        int score = calculateScore(difficulty, wordGuessed);
        if (wordGuessed)
        {
            printf("Your score for this round: %d\n", score);

            // Add score to the currentScore
            currentScore += score;

            // Update high score if necessary
            if (currentScore > highScore)
            {
                printf("New High Score! Updating...\n");
                updateHighScore("highscore.txt", currentScore);
                highScore = currentScore;
            }
        }

        printf("Total Score so far: %d\n", currentScore); // Show cumulative score after each round
    } while (playAgain()); // If the user chooses to play again, the game will restart

    return 0;
}

void displayWord(const char word[], const bool guessed[])
{
    printf("Word: ");
    for (int i = 0; word[i] != '\0'; i++)
    {
        if (guessed[word[i] - 'a'])
        {
            printf("%c ", word[i]);
        }
        else
        {
            printf("_ ");
        }
    }
    printf("\n");
}

void drawHangman(int tries)
{
    const char *hangmanParts[] = {
        "     _________",
        "    |         |",
        "    |         O",
        "    |        /|\\",
        "    |        / \\",
    };

    printf("\n");
    for (int i = 0; i <= tries; i++)
    {
        printf("%s\n", hangmanParts[i]);
    }
}

int loadWords(const char *filename, struct WordWithHint wordList[], int maxWords)
{
    FILE *file = fopen(filename, "r");
    if (!file)
    {
        printf("Error: Could not open file %s\n", filename);
        return 0;
    }

    int count = 0;
    char line[3 * MAX_WORD_LENGTH];
    while (fgets(line, sizeof(line), file) && count < maxWords)
    {
        char *word = strtok(line, "|");
        char *hint = strtok(NULL, "|");
        char *difficulty = strtok(NULL, "\n");

        if (word && hint && difficulty)
        {
            strncpy(wordList[count].word, word, MAX_WORD_LENGTH - 1);
            strncpy(wordList[count].hint, hint, MAX_WORD_LENGTH - 1);
            strncpy(wordList[count].difficulty, difficulty, sizeof(wordList[count].difficulty) - 1);
            count++;
        }
    }

    fclose(file);
    return count;
}

int filterWordsByDifficulty(struct WordWithHint wordList[], int wordCount, struct WordWithHint filteredList[], const char *difficulty)
{
    int count = 0;
    for (int i = 0; i < wordCount; i++)
    {
        if (strcmp(wordList[i].difficulty, difficulty) == 0)
        {
            filteredList[count++] = wordList[i];
        }
    }
    return count;
}

bool playAgain()
{
    char response;
    printf("Do you want to play again? (y/n): ");
    scanf(" %c", &response);

    // Ensure the response is 'y' or 'n'
    if (tolower(response) != 'y' && tolower(response) != 'n')
    {
        printf("Invalid input. Please enter 'y' for Yes or 'n' for No.\n");
        return playAgain(); // Recursively ask until valid input is received
    }

    return (tolower(response) == 'y');
}

int readHighScore(const char *filename)
{
    FILE *file = fopen(filename, "r");
    if (!file)
    {
        return 0; // Default high score if the file doesn't exist
    }

    int highScore;
    fscanf(file, "%d", &highScore);
    fclose(file);

    return highScore;
}

void updateHighScore(const char *filename, int score)
{
    FILE *file = fopen(filename, "w");
    if (!file)
    {
        printf("Error: Could not open file %s for writing.\n", filename);
        return;
    }

    fprintf(file, "%d\n", score);
    fclose(file);
}

int calculateScore(const char *difficulty, bool wordGuessed)
{
    if (!wordGuessed)
    {
        return 0; // No score if the word wasn't guessed
    }

    int score = 0;
    if (strcmp(difficulty, "easy") == 0)
    {
        score = 2;
    }
    else if (strcmp(difficulty, "medium") == 0)
    {
        score = (int)pow(2, 2); // 2^2 for medium
    }
    else if (strcmp(difficulty, "hard") == 0)
    {
        score = (int)pow(2, 3); // 2^3 for hard
    }
    return score;
}