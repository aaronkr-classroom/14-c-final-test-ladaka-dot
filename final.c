#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>   // _countof

#define NAME_LEN 32
#define DEFAULT_FILE "students.dat"

typedef struct {
    char name[NAME_LEN];
    int kor;
    int eng;
    int math;
} Student;

typedef struct Node {
    Student s;
    struct Node* next;
} Node;

/* ================== 연결 리스트 유틸 ================== */
void clear_list(Node** head, Node** tail) {
    Node* p = *head;
    while (p) {
        Node* tmp = p;
        p = p->next;
        free(tmp);
    }
    *head = NULL;
    *tail = NULL;
}

void append_student(Node** head, Node** tail, Student st) {
    Node* newNode = (Node*)malloc(sizeof(Node));
    if (!newNode) {
        printf("메모리 할당 실패!\n");
        return;
    }
    newNode->s = st;
    newNode->next = NULL;

    if (*head == NULL) {
        *head = newNode;
        *tail = newNode;
    }
    else {
        (*tail)->next = newNode;
        *tail = newNode;
    }
}

int list_size(Node* head) {
    int cnt = 0;
    for (Node* p = head; p; p = p->next) cnt++;
    return cnt;
}

/* ================== 파일 입출력(.dat) ================== */
void load_from_file(Node** head, Node** tail, const char* filename) {
    FILE* fp = fopen(filename, "rb");
    if (!fp) {
        printf("파일 열기 실패: %s (파일이 없을 수도 있음)\n", filename);
        return;
    }

    clear_list(head, tail);

    Student st;
    int count = 0;
    while (fread(&st, sizeof(Student), 1, fp) == 1) {
        append_student(head, tail, st);
        count++;
    }
    fclose(fp);

    printf("파일에서 %d명의 데이터를 읽었습니다.\n", count);
}

void save_to_file(Node* head, const char* filename) {
    FILE* fp = fopen(filename, "wb");
    if (!fp) {
        printf("파일 열기 실패: %s\n", filename);
        return;
    }

    int count = 0;
    for (Node* p = head; p; p = p->next) {
        fwrite(&(p->s), sizeof(Student), 1, fp);
        count++;
    }
    fclose(fp);

    printf("파일에 %d명의 데이터를 저장했습니다.\n", count);
}

/* ================== 성적 출력(총점/평균/등수) ================== */
int total_score(const Student* s) {
    return s->kor + s->eng + s->math;
}

int cmp_total_desc(const void* a, const void* b) {
    const Node* na = *(const Node**)a;
    const Node* nb = *(const Node**)b;
    int ta = total_score(&na->s);
    int tb = total_score(&nb->s);
    return (tb - ta); // 내림차순
}

void print_report(Node* head) {
    int n = list_size(head);
    if (n == 0) {
        printf("저장된 학생 데이터가 없습니다.\n");
        return;
    }

    Node** arr = (Node**)malloc(sizeof(Node*) * n);
    if (!arr) {
        printf("메모리 할당 실패!\n");
        return;
    }

    int i = 0;
    for (Node* p = head; p; p = p->next) {
        arr[i++] = p;
    }

    qsort(arr, n, sizeof(Node*), cmp_total_desc);

    int* rank = (int*)malloc(sizeof(int) * n);
    if (!rank) {
        free(arr);
        printf("메모리 할당 실패!\n");
        return;
    }

    // 경쟁 순위(동점이면 같은 등수, 다음 등수 건너뜀)
    rank[0] = 1;
    for (i = 1; i < n; i++) {
        int prevTotal = total_score(&arr[i - 1]->s);
        int curTotal = total_score(&arr[i]->s);
        if (curTotal == prevTotal) rank[i] = rank[i - 1];
        else rank[i] = i + 1;
    }

    printf("\n====================================================\n");
    printf("%-10s %5s %5s %5s %6s %7s %5s\n",
        "이름", "국어", "영어", "수학", "총점", "평균", "등수");
    printf("----------------------------------------------------\n");

    for (i = 0; i < n; i++) {
        Student* s = &arr[i]->s;
        int tot = total_score(s);
        double avg = tot / 3.0;

        printf("%-10s %5d %5d %5d %6d %7.2f %4d등\n",
            s->name, s->kor, s->eng, s->math, tot, avg, rank[i]);
    }
    printf("====================================================\n\n");

    free(rank);
    free(arr);
}

/* ================== 입력(scanf_s) ================== */
static void flush_stdin_line(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {}
}

Student input_student(void) {
    Student st;
    memset(&st, 0, sizeof(Student));

    printf("학생 이름(공백 없이): ");
    scanf_s("%31s", st.name, (unsigned)_countof(st.name));

    printf("국어 점수: ");
    scanf_s("%d", &st.kor);

    printf("영어 점수: ");
    scanf_s("%d", &st.eng);

    printf("수학 점수: ");
    scanf_s("%d", &st.math);

    return st;
}

int main(void) {
    Node* head = NULL;
    Node* tail = NULL;

    int menu = 0;
    char filename[256];

    while (1) {
        printf("[Menu]\n");
        printf("1. .dat 파일에서 데이터 읽기\n");
        printf("2. 추가 학생 정보 입력\n");
        printf("3. .dat 파일 저장\n");
        printf("4. 성적 확인 (평균 계산 등)\n");
        printf("5. 종료\n");
        printf("-------------------\n");
        printf("선택(1~5): ");

        if (scanf_s("%d", &menu) != 1) {
            printf("입력이 올바르지 않습니다.\n");
            flush_stdin_line();
            continue;
        }

        if (menu == 1) {
            printf("읽을 파일명(예: %s): ", DEFAULT_FILE);
            scanf_s("%255s", filename, (unsigned)_countof(filename));
            load_from_file(&head, &tail, filename);
        }
        else if (menu == 2) {
            Student st = input_student();
            append_student(&head, &tail, st);
            printf("학생 정보가 추가되었습니다.\n");
        }
        else if (menu == 3) {
            printf("저장할 파일명(예: %s): ", DEFAULT_FILE);
            scanf_s("%255s", filename, (unsigned)_countof(filename));
            save_to_file(head, filename);
        }
        else if (menu == 4) {
            print_report(head);
        }
        else if (menu == 5) {
            printf("프로그램을 종료합니다.\n");
            break;
        }
        else {
            printf("1~5 중에서 선택하세요.\n");
        }
    }

    clear_list(&head, &tail);
    return 0;
}
