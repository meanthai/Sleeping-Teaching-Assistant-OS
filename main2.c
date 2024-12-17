#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <semaphore.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#define SO_SINH_VIEN_TOI_DA 100
#define SO_GHE_TOI_DA 3
#define SO_LAN_LAP_TOI_DA 10

pthread_t *cacSinhVien;
pthread_t troGiang;
int soSinhVienDangCho = 0;
bool troGiangNgu = true;
bool chuongTrinhDangChay = true;

sem_t semSinhVien;
sem_t semTroGiang;
pthread_mutex_t khoaGhe = PTHREAD_MUTEX_INITIALIZER;

void *hoatDongTroGiang(void *arg) {
    int tongSoSinhVien = *(int*)arg;
    int soSinhVienDaHoTro = 0;

    while (chuongTrinhDangChay && soSinhVienDaHoTro < tongSoSinhVien * SO_LAN_LAP_TOI_DA) {
        printf("Trợ giảng đang ngủ...\n");
        sem_wait(&semTroGiang);

        if (!chuongTrinhDangChay) break;

        while (soSinhVienDangCho > 0) {
            pthread_mutex_lock(&khoaGhe);
            if (soSinhVienDangCho > 0) {
                soSinhVienDangCho--;
                printf("Trợ giảng đang giúp sinh viên. Hàng đợi còn: %d\n", soSinhVienDangCho);
            }
            pthread_mutex_unlock(&khoaGhe);

            sleep(2);

            sem_post(&semSinhVien);
            soSinhVienDaHoTro++;
        }
    }

    chuongTrinhDangChay = false;
    for (int i = 0; i < soSinhVienDangCho; i++) {
        sem_post(&semSinhVien);
    }

    printf("Trợ giảng kết thúc sau %d lần hỗ trợ.\n", soSinhVienDaHoTro);
    return NULL;
}

void *hoatDongSinhVien(void *arg) {
    long maSinhVien = (long)arg;
    int soLanLap = 0;

    while (chuongTrinhDangChay && soLanLap < SO_LAN_LAP_TOI_DA) {
        int thoiGianLapTrinh = rand() % 5 + 1;
        printf("Sinh viên %ld đang lập trình trong %d giây\n", maSinhVien, thoiGianLapTrinh);
        sleep(thoiGianLapTrinh);

        pthread_mutex_lock(&khoaGhe);
        if (soSinhVienDangCho < SO_GHE_TOI_DA) {
            soSinhVienDangCho++;
            printf("Sinh viên %ld đợi. Số lượng trong hàng: %d\n", maSinhVien, soSinhVienDangCho);
            
            if (soSinhVienDangCho == 1) {
                sem_post(&semTroGiang);
            }
            pthread_mutex_unlock(&khoaGhe);

            sem_wait(&semSinhVien);
            printf("Sinh viên %ld đã được hỗ trợ và rời đi.\n", maSinhVien);
        } else {
            pthread_mutex_unlock(&khoaGhe);
            printf("Sinh viên %ld sẽ quay lại sau (hết ghế).\n", maSinhVien);
            usleep(500000);
        }

        soLanLap++;
    }

    return NULL;
}

int main(int argc, char* argv[]) {
    int soLuongSinhVien;
    
    if (argc < 2) {
        printf("Không nhập số sinh viên. Mặc định 5 sinh viên.\n");
        soLuongSinhVien = 5;
    } else {
        soLuongSinhVien = atoi(argv[1]);
        if (soLuongSinhVien <= 0 || soLuongSinhVien > SO_SINH_VIEN_TOI_DA) {
            fprintf(stderr, "Số lượng sinh viên không hợp lệ. Dùng mặc định 5.\n");
            soLuongSinhVien = 5;
        }
    }

    srand(time(NULL));

    sem_init(&semSinhVien, 0, 0);
    sem_init(&semTroGiang, 0, 0);

    cacSinhVien = malloc(sizeof(pthread_t) * soLuongSinhVien);
    if (cacSinhVien == NULL) {
        fprintf(stderr, "Lỗi cấp phát bộ nhớ\n");
        return 1;
    }

    if (pthread_create(&troGiang, NULL, hoatDongTroGiang, &soLuongSinhVien) != 0) {
        fprintf(stderr, "Lỗi tạo luồng trợ giảng\n");
        free(cacSinhVien);
        return 1;
    }

    for (int i = 0; i < soLuongSinhVien; i++) {
        if (pthread_create(&cacSinhVien[i], NULL, hoatDongSinhVien, (void*)(long)i) != 0) {
            fprintf(stderr, "Lỗi tạo luồng sinh viên %d\n", i);
            chuongTrinhDangChay = false;
            break;
        }
    }

    pthread_join(troGiang, NULL);
    for (int i = 0; i < soLuongSinhVien; i++) {
        pthread_join(cacSinhVien[i], NULL);
    }

    free(cacSinhVien);
    sem_destroy(&semSinhVien);
    sem_destroy(&semTroGiang);
    pthread_mutex_destroy(&khoaGhe);

    return 0;
}
