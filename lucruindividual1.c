#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE 1024
#define MAX_RECORDS 10000 // Ajustează dacă fișierul este mai mare
#define MAX_STRING 100

// Structura pentru o singură tranzacție
typedef struct {
    char date[15];
    int product_id;
    char product_name[MAX_STRING];
    char category[MAX_STRING];
    char subcategory[MAX_STRING];
    double unit_price;
    int quantity;
    char country[MAX_STRING];
    char city[MAX_STRING];
    double total_revenue; // Calculat ca: unit_price * quantity
} SaleRecord;

// Structură generică pentru agregări (simulează un dicționar/map)
typedef struct {
    char key[MAX_STRING];
    double revenue;
} AggData;

// Funcție ajutătoare pentru a extrage doar Anul și Luna (ex: 2018-08) din data (YYYY-MM-DD)
void get_year_month(const char* full_date, char* year_month) {
    strncpy(year_month, full_date, 7);
    year_month[7] = '\0';
}

// Funcție pentru a găsi sau adăuga o cheie într-un tablou de agregare
int find_or_add(AggData* arr, int* size, const char* key) {
    for (int i = 0; i < *size; i++) {
        if (strcmp(arr[i].key, key) == 0) {
            return i;
        }
    }
    strcpy(arr[*size].key, key);
    arr[*size].revenue = 0.0;
    (*size)++;
    return (*size) - 1;
}

// Funcție pentru sortarea descrescătoare a agregărilor
int compare_agg(const void* a, const void* b) {
    AggData* aggA = (AggData*)a;
    AggData* aggB = (AggData*)b;
    if (aggB->revenue > aggA->revenue) return 1;
    if (aggB->revenue < aggA->revenue) return -1;
    return 0;
}

int main() {
    FILE *file = fopen("sales.csv", "r");
    if (!file) {
        printf("Eroare: Nu am putut deschide fisierul sales.csv\n");
        return 1;
    }

    SaleRecord* sales = malloc(MAX_RECORDS * sizeof(SaleRecord));
    int count = 0;
    char line[MAX_LINE];

    // Citim antetul (header-ul) și îl ignorăm
    fgets(line, MAX_LINE, file);

    // Parsarea liniilor din CSV
    while (fgets(line, MAX_LINE, file) && count < MAX_RECORDS) {
        // Eliminăm newline-ul
        line[strcspn(line, "\n")] = 0; 

        char *token = strtok(line, ",");
        if (!token) continue;
        strcpy(sales[count].date, token);

        token = strtok(NULL, ",");
        sales[count].product_id = token ? atoi(token) : 0;

        token = strtok(NULL, ",");
        strcpy(sales[count].product_name, token ? token : "");

        token = strtok(NULL, ",");
        strcpy(sales[count].category, token ? token : "");

        token = strtok(NULL, ",");
        strcpy(sales[count].subcategory, token ? token : "");

        token = strtok(NULL, ",");
        sales[count].unit_price = token ? atof(token) : 0.0;

        token = strtok(NULL, ",");
        sales[count].quantity = token ? atoi(token) : 0;

        token = strtok(NULL, ",");
        strcpy(sales[count].country, token ? token : "");

        token = strtok(NULL, ",");
        strcpy(sales[count].city, token ? token : "");

        sales[count].total_revenue = sales[count].unit_price * sales[count].quantity;
        count++;
    }
    fclose(file);

    printf("=== ANALIZA VANZARILOR (Total inregistrari: %d) ===\n\n", count);

    // --- 1. Venitul total generat în fiecare lună ---
    AggData monthly_revenue[1000];
    int num_months = 0;
    for (int i = 0; i < count; i++) {
        char month[10];
        get_year_month(sales[i].date, month);
        int idx = find_or_add(monthly_revenue, &num_months, month);
        monthly_revenue[idx].revenue += sales[i].total_revenue;
    }
    qsort(monthly_revenue, num_months, sizeof(AggData), compare_agg);
    
    printf("1. Venitul total generat in fiecare luna (Top 5 luni):\n");
    for (int i = 0; i < num_months && i < 5; i++) {
        printf("   - Luna %s: %.2f $\n", monthly_revenue[i].key, monthly_revenue[i].revenue);
    }
    printf("\n");

    // --- 2. Primele 5 produse cel mai bine vândute (după venit) ---
    AggData product_revenue[5000];
    int num_products = 0;
    for (int i = 0; i < count; i++) {
        int idx = find_or_add(product_revenue, &num_products, sales[i].product_name);
        product_revenue[idx].revenue += sales[i].total_revenue;
    }
    qsort(product_revenue, num_products, sizeof(AggData), compare_agg);

    printf("2. Primele 5 produse dupa venitul generat:\n");
    for (int i = 0; i < 5 && i < num_products; i++) {
        printf("   - Produs: '%s' | Venit: %.2f $\n", product_revenue[i].key, product_revenue[i].revenue);
    }
    printf("\n");

    // --- 3. Distribuția vânzărilor pe categorii de produse ---
    AggData category_revenue[100];
    int num_categories = 0;
    for (int i = 0; i < count; i++) {
        int idx = find_or_add(category_revenue, &num_categories, sales[i].category);
        category_revenue[idx].revenue += sales[i].total_revenue;
    }
    qsort(category_revenue, num_categories, sizeof(AggData), compare_agg);

    printf("3. Distributia vanzarilor pe categorii:\n");
    for (int i = 0; i < num_categories; i++) {
        printf("   - Categorie: '%s' | Venit: %.2f $\n", category_revenue[i].key, category_revenue[i].revenue);
    }
    printf("\n");

    // --- 4. Orașele cu cele mai mari vânzări (Afișare generală Top 5 orașe) ---
    AggData city_revenue[5000];
    int num_cities = 0;
    for (int i = 0; i < count; i++) {
        // Formăm o cheie combinată: "Tara - Oras"
        char location[MAX_STRING * 2];
        snprintf(location, sizeof(location), "%s - %s", sales[i].country, sales[i].city);
        int idx = find_or_add(city_revenue, &num_cities, location);
        city_revenue[idx].revenue += sales[i].total_revenue;
    }
    qsort(city_revenue, num_cities, sizeof(AggData), compare_agg);

    printf("4. Top 5 orase cu cele mai mari vanzari global:\n");
    for (int i = 0; i < 5 && i < num_cities; i++) {
        printf("   - Locatie: %s | Venit: %.2f $\n", city_revenue[i].key, city_revenue[i].revenue);
    }
    printf("\n");

    // Eliberăm memoria alocată
    free(sales);
    getchar();
    
    return 0;
}
