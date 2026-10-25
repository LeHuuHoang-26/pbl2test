#include "StatisticController.h"
#include "DoctorController.h"

int StatisticController::patientCount(const LinkList<Patient> *items) {
    return items ? items->count : 0;
}

int StatisticController::doctorCount(const LinkList<Doctor> *items) {
    return items ? items->count : 0;
}

int StatisticController::appointmentCount(const LinkList<Appointment> *items) {
    return items ? items->count : 0;
}

int StatisticController::pendingAppointmentCount(const LinkList<Appointment> *items) {
    if (!items || !items->getHeader()) return 0;
    int c = 0;
    auto *n = items->getHeader()->next;
    while (n && n->next) {
        if (!n->info.isDefault() && n->info.getState() == 0) {
            c++;
        }
        n = n->next;
    }
    return c;
}

double StatisticController::totalRevenue(const LinkList<Appointment> *appts, const LinkList<Doctor> *docs) {
    if (!appts || !appts->getHeader() || !docs) return 0.0;
    double total = 0.0;
    auto *docList = const_cast<LinkList<Doctor>*>(docs);
    auto *n = appts->getHeader()->next;
    while (n && n->next) {
        if (!n->info.isDefault() && n->info.getState() == 1) { // 1 = Da kham
            Doctor *d = DoctorController::findById(docList, n->info.getDoctorId());
            if (d) {
                total += d->getFee();
            }
        }
        n = n->next;
    }
    return total;
}

double StatisticController::todayRevenue(const LinkList<Appointment> *appts, const LinkList<Doctor> *docs, int day, int month, int year) {
    if (!appts || !appts->getHeader() || !docs) return 0.0;
    double total = 0.0;
    auto *docList = const_cast<LinkList<Doctor>*>(docs);
    auto *n = appts->getHeader()->next;
    while (n && n->next) {
        if (!n->info.isDefault() && n->info.getState() == 1) {
            Date dt = n->info.getDateGet();
            if (dt.getDay() == day && dt.getMonth() == month && dt.getYear() == year) {
                Doctor *d = DoctorController::findById(docList, n->info.getDoctorId());
                if (d) {
                    total += d->getFee();
                }
            }
        }
        n = n->next;
    }
    return total;
}

double StatisticController::completionRate(const LinkList<Appointment> *appts, int currentDay, int currentMonth, int currentYear, int currentHour, int currentMinute, int &completedCount, int &totalUntilNow) {
    completedCount = 0;
    totalUntilNow = 0;
    if (!appts || !appts->getHeader() || appts->count == 0) return 0.0;

    auto *n = appts->getHeader()->next;
    while (n && n->next) {
        if (!n->info.isDefault()) {
            int st = n->info.computeEffectiveState(currentDay, currentMonth, currentYear, currentHour, currentMinute);
            // Các ca tính đến thời điểm hiện tại: Đã khám (1), Trễ hẹn (-1)
            if (st == 1 || st == -1) {
                totalUntilNow++;
                if (st == 1) {
                    completedCount++;
                }
            }
        }
        n = n->next;
    }

    if (totalUntilNow == 0) return 0.0;
    return (completedCount * 100.0) / totalUntilNow;
}

double StatisticController::completionRate(const LinkList<Appointment> *appts) {
    int comp = 0, tot = 0;
    Date now = Date::now();
    return completionRate(appts, now.getDay(), now.getMonth(), now.getYear(), now.getHour(), now.getMinute(), comp, tot);
}

void StatisticController::appointmentStatusCounts(const LinkList<Appointment> *appts, int &completed, int &pending, int &delayed, int currentDay, int currentMonth, int currentYear, int currentHour, int currentMinute) {
    completed = 0;
    pending = 0;
    delayed = 0;
    if (!appts || !appts->getHeader()) return;

    auto *n = appts->getHeader()->next;
    while (n && n->next) {
        if (!n->info.isDefault()) {
            int st = n->info.computeEffectiveState(currentDay, currentMonth, currentYear, currentHour, currentMinute);
            switch (st) {
                case 1: completed++; break;
                case 0: pending++; break;
                case -1: delayed++; break;
                default: break;
            }
        }
        n = n->next;
    }
}

void StatisticController::appointmentStatusCounts(const LinkList<Appointment> *appts, int &completed, int &pending, int &delayed) {
    Date now = Date::now();
    appointmentStatusCounts(appts, completed, pending, delayed, now.getDay(), now.getMonth(), now.getYear(), now.getHour(), now.getMinute());
}

void StatisticController::todayAppointmentCounts(const LinkList<Appointment> *appts, int day, int month, int year, int &totalToday, int &completedToday, int &pendingToday) {
    totalToday = 0;
    completedToday = 0;
    pendingToday = 0;
    if (!appts || !appts->getHeader()) return;

    auto *n = appts->getHeader()->next;
    while (n && n->next) {
        if (!n->info.isDefault()) {
            Date dt = n->info.getDateGet();
            if (dt.getDay() == day && dt.getMonth() == month && dt.getYear() == year) {
                totalToday++;
                if (n->info.getState() == 1) completedToday++;
                else if (n->info.getState() == 0) pendingToday++;
            }
        }
        n = n->next;
    }
}

void StatisticController::patientGenderCounts(const LinkList<Patient> *patients, int &male, int &female) {
    male = 0;
    female = 0;
    if (!patients || !patients->getHeader()) return;

    auto *n = patients->getHeader()->next;
    while (n && n->next) {
        if (!n->info.isDefault()) {
            if (n->info.getGender()) male++;
            else female++;
        }
        n = n->next;
    }
}

void StatisticController::patientAgeDistribution(const LinkList<Patient> *patients, int &under18, int &from18to45, int &from46to60, int &over60) {
    under18 = 0;
    from18to45 = 0;
    from46to60 = 0;
    over60 = 0;
    if (!patients || !patients->getHeader()) return;

    auto *n = patients->getHeader()->next;
    while (n && n->next) {
        if (!n->info.isDefault()) {
            int age = n->info.getAge();
            if (age < 18) under18++;
            else if (age <= 45) from18to45++;
            else if (age <= 60) from46to60++;
            else over60++;
        }
        n = n->next;
    }
}

int StatisticController::getTopDoctors(const LinkList<Appointment> *appts, const LinkList<Doctor> *docs, DoctorLeaderboardItem outList[], int maxCount) {
    if (!docs || !docs->getHeader() || maxCount <= 0) return 0;

    int totalDocs = docs->count;
    if (totalDocs <= 0) return 0;

    // Allocate buffer for all doctors
    DoctorLeaderboardItem *allItems = new DoctorLeaderboardItem[totalDocs];
    int idx = 0;

    auto *dn = docs->getHeader()->next;
    while (dn && dn->next && idx < totalDocs) {
        if (!dn->info.isDefault()) {
            allItems[idx].doctorId = dn->info.getID();
            allItems[idx].doctorName = dn->info.getfullName();
            allItems[idx].specialtyId = dn->info.getSpId();
            allItems[idx].roomId = dn->info.getRoomId();
            allItems[idx].rating = dn->info.getRating();
            allItems[idx].appointmentCount = 0;
            idx++;
        }
        dn = dn->next;
    }
    int validDoctors = idx;

    // Count appointments for each doctor
    if (appts && appts->getHeader()) {
        auto *an = appts->getHeader()->next;
        while (an && an->next) {
            if (!an->info.isDefault()) {
                std::string did = an->info.getDoctorId();
                for (int i = 0; i < validDoctors; ++i) {
                    if (allItems[i].doctorId == did) {
                        allItems[i].appointmentCount++;
                        break;
                    }
                }
            }
            an = an->next;
        }
    }

    // Sort descending by appointmentCount (simple bubble sort - no forbidden STL headers)
    for (int i = 0; i < validDoctors - 1; ++i) {
        for (int j = 0; j < validDoctors - i - 1; ++j) {
            if (allItems[j].appointmentCount < allItems[j + 1].appointmentCount) {
                DoctorLeaderboardItem tmp = allItems[j];
                allItems[j] = allItems[j + 1];
                allItems[j + 1] = tmp;
            }
        }
    }

    int copyCount = (validDoctors < maxCount) ? validDoctors : maxCount;
    for (int i = 0; i < copyCount; ++i) {
        outList[i] = allItems[i];
    }

    delete[] allItems;
    return copyCount;
}

int StatisticController::getSpecialtyDemand(const LinkList<Appointment> *appts, const LinkList<Doctor> *docs, SpecialtyDemandItem outList[], int maxCount) {
    if (!appts || !appts->getHeader() || !docs || maxCount <= 0) return 0;

    auto *docList = const_cast<LinkList<Doctor>*>(docs);
    SpecialtyDemandItem items[20];
    int uniqueCount = 0;

    auto *an = appts->getHeader()->next;
    while (an && an->next) {
        if (!an->info.isDefault()) {
            Doctor *d = DoctorController::findById(docList, an->info.getDoctorId());
            if (d) {
                std::string spId = d->getSpId();
                int foundIdx = -1;
                for (int i = 0; i < uniqueCount; ++i) {
                    if (items[i].spId == spId) {
                        foundIdx = i;
                        break;
                    }
                }
                if (foundIdx >= 0) {
                    items[foundIdx].count++;
                } else if (uniqueCount < 20) {
                    items[uniqueCount].spId = spId;
                    items[uniqueCount].count = 1;
                    uniqueCount++;
                }
            }
        }
        an = an->next;
    }

    // Sort descending by count
    for (int i = 0; i < uniqueCount - 1; ++i) {
        for (int j = 0; j < uniqueCount - i - 1; ++j) {
            if (items[j].count < items[j + 1].count) {
                SpecialtyDemandItem tmp = items[j];
                items[j] = items[j + 1];
                items[j + 1] = tmp;
            }
        }
    }

    int copyCount = (uniqueCount < maxCount) ? uniqueCount : maxCount;
    for (int i = 0; i < copyCount; ++i) {
        outList[i] = items[i];
    }
    return copyCount;
}

void StatisticController::getWeeklyTrend(const LinkList<Appointment> *appts, int currentDay, int currentMonth, int currentYear, int outCounts[7]) {
    for (int i = 0; i < 7; ++i) outCounts[i] = 0;
    if (!appts || !appts->getHeader()) return;

    Date today(currentDay, currentMonth, currentYear);

    // Compute dates for the last 7 days: day[0] = 6 days ago, ..., day[6] = today
    Date days[7];
    for (int i = 0; i < 7; ++i) {
        days[i] = today - (6 - i);
    }

    auto *an = appts->getHeader()->next;
    while (an && an->next) {
        if (!an->info.isDefault()) {
            Date dt = an->info.getDateGet();
            for (int i = 0; i < 7; ++i) {
                if (dt.getDay() == days[i].getDay() &&
                    dt.getMonth() == days[i].getMonth() &&
                    dt.getYear() == days[i].getYear()) {
                    outCounts[i]++;
                    break;
                }
            }
        }
        an = an->next;
    }
}
