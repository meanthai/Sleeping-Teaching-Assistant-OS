#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <semaphore.h>
#include <stdlib.h>
#include <stdio.h>

pthread_t *SinhVien;
pthread_t TroGiang;

int SoGhe = 0;
int ChiSoHienTai = 0;

sem_t NgungNgu;
sem_t SemSinhVien;
sem_t Ghe[3];
pthread_mutex_t KhoaGhe;

void *HoatDongTroGiang();
void *HoatDongSinhVien(void *id);

int main(int argc, char* argv[]) {
    int soLuongSinhVien;
    int id;
    srand(time(NULL));

    sem_init(&NgungNgu, 0, 0);
    sem_init(&SemSinhVien, 0, 0);
    for (id = 0; id < 3; ++id)
        sem_init(&Ghe[id], 0, 0);

    pthread_mutex_init(&KhoaGhe, NULL);

    if (argc < 2) {
        printf("Khong nhap so sinh vien. Mac dinh tao 5 sinh vien.\n");
        soLuongSinhVien = 5;
    } else {
        printf("So sinh vien nhap vao: %d\n", atoi(argv[1]));
        soLuongSinhVien = atoi(argv[1]);
    }

    SinhVien = (pthread_t *)malloc(sizeof(pthread_t) * soLuongSinhVien);

    pthread_create(&TroGiang, NULL, HoatDongTroGiang, NULL);
    for (id = 0; id < soLuongSinhVien; id++)
        pthread_create(&SinhVien[id], NULL, HoatDongSinhVien, (void *)(long)id);

    pthread_join(TroGiang, NULL);
    for (id = 0; id < soLuongSinhVien; id++)
        pthread_join(SinhVien[id], NULL);

    free(SinhVien);
    return 0;
}

void *HoatDongTroGiang() {
    while (1) {
        sem_wait(&NgungNgu);
        printf("~~~~~~~~~~ Tro giang da bi danh thuc boi sinh vien. ~~~~~~~~~~\n");

        while (1) {
            pthread_mutex_lock(&KhoaGhe);
            if (SoGhe == 0) {
                pthread_mutex_unlock(&KhoaGhe);
                break;
            }
            sem_post(&Ghe[ChiSoHienTai]);
            SoGhe--;
            printf("Sinh vien roi ghe. So ghe con lai: %d\n", 3 - SoGhe);
            ChiSoHienTai = (ChiSoHienTai + 1) % 3;
            pthread_mutex_unlock(&KhoaGhe);

            printf("\t Tro giang dang giup sinh vien.\n");
            sleep(5);
            sem_post(&SemSinhVien);
            usleep(1000);
        }
    }
}

void *HoatDongSinhVien(void *id) {
    int ThoiGianLapTrinh;

    while (1) {
        printf("Sinh vien %ld dang lap trinh.\n", (long)id);
        ThoiGianLapTrinh = rand() % 10 + 1;
        sleep(ThoiGianLapTrinh);

        printf("Sinh vien %ld can giup do tu tro giang.\n", (long)id);

        pthread_mutex_lock(&KhoaGhe);
        int soLuong = SoGhe;
        pthread_mutex_unlock(&KhoaGhe);

        if (soLuong < 3) {
            if (soLuong == 0)
                sem_post(&NgungNgu);
            else
                printf("Sinh vien %ld ngoi ghe doi tro giang.\n", (long)id);

            pthread_mutex_lock(&KhoaGhe);
            int chiSo = (ChiSoHienTai + SoGhe) % 3;
            SoGhe++;
            printf("Sinh vien da ngoi ghe. So ghe con lai: %d\n", 3 - SoGhe);
            pthread_mutex_unlock(&KhoaGhe);

            sem_wait(&Ghe[chiSo]);
            printf("\t Sinh vien %ld dang duoc tro giang giup.\n", (long)id);
            sem_wait(&SemSinhVien);
            printf("Sinh vien %ld roi phong tro giang.\n", (long)id);
        } else {
            printf("Sinh vien %ld se quay lai sau.\n", (long)id);
        }
    }
}
