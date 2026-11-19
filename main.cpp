#include <QCoreApplication>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonParseError>

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <map>
using namespace std;
// ENUM GENDER --> feste auswahl an möglichkeiten 3 geschlecht json datei
enum Gender
{
    male,
    female,
    nonBinary
};
// KLASSE STAY
class Stay
{
private:
    long patientID;
    long stayID;
    string admissionDate;
    string dischargeDate;
    string mainDiagnosisCode;
    string mainDiagnosisText;
public: //Konstruktor eird beim erstellen eines neuen stay objekts aufgerufen, er bekommt die daten als paraneter und speichert als attribute
Stay(long pID, long sID, string aDate, string dDate,
     string diagnosisCode, string diagnosisText)
{
    patientID = pID;
    stayID = sID;
    admissionDate = aDate;
    dischargeDate = dDate;
    mainDiagnosisCode = diagnosisCode;
    mainDiagnosisText = diagnosisText;
}

string showDetails() //nimmt die gespeicherten informationen eines krankenhausaufenthalts und macht daraus einen text
{
    string result;

    result += to_string(stayID) + ": von " + admissionDate; //+= hängt an bisherigen inhalt an

    if (dischargeDate == "") //prüft ob ebtlassungsdatum leer ist weil vlt liegt patient noch im krankenhaus
        result += " bis noch nicht entlassen\n";
    else
        result += " bis " + dischargeDate + "\n";

    result += "Aufnahmediagnose: ";
    result += mainDiagnosisCode + " " + mainDiagnosisText + "\n";

    return result;
}
};
// KLASSE PATIENT
class Patient
{
private:
    long patientID;
    string birthDate;
    string firstName;
    string lastName;
    string insuranceNumber;
    Gender gender;
    vector<Stay> stays; //liste für mehrere krankenhausaufenthalte für einen patienten
public:
Patient(long pID, string bDate, string fName,   //wie stay erstellt neue patienten objekt
        string lName, string insurance, Gender g)
{
    patientID = pID;
    birthDate = bDate;
    firstName = fName;
    lastName = lName;
    insuranceNumber = insurance;
    gender = g;
}

void addStay(Stay stay) //wir übergeben einen krankenhausaufenthalt unf fügen das liste hinzu
{
    stays.push_back(stay);
}

string getInsuranceNumber()
{
    return insuranceNumber;
}

long getPatientID()
{
    return patientID;
}

string showOverview()   //erstellt eine komplette übersicht über patient
{
    string result;//text wird jetzt geschrieben

    result += "--------------------------------------\n";
    result += "Patient*in " + to_string(patientID) + ": ";
    result += firstName + " " + lastName + "\n";

    result += "Versicherungsnummer: ";
    result += insuranceNumber + "\n";

    result += "Geschlecht: ";

    if (gender == male)
        result += "maennlich";
    else if (gender == female)
        result += "weiblich";
    else
        result += "nicht binaer";

    result += ", Geburtsdatum: " + birthDate + "\n";
    result += "Aufenthalte:\n";

    for (Stay stay : stays) //die schleife geht die ganze liste im vector durch
    {
        result += stay.showDetails();
    }

    return result;
}
};
// KLASSE HOSPITAL verwaltet alle patienten und liest dateien ein
class Hospital
{
private:
    string name;
    vector<Patient> patients;   //liste in der alle patienten gespeichert werden
    map<string, string> icdCodes; //map verbindet zwei werte; bei uns schlüssel=ICD Code Wert= Diagnosetext
    bool icdLoaded = false;
public:
Hospital(string hospitalName)
{
    name = hospitalName;
}
//klasse hospital verwaltet alle patienten in einem vector und speichert die ICD codes mit den zugehörigen diagnosetexten in einer map


// ICD-DATEI EINLESEN

void readICDData(string filename) //liest die datei icdCodes.txt ein und speichert die ICD codes mit den passenden diagnosetexten in map
{
    ifstream file(filename);    //datei wird versucht zu öffnen

    if (!file.is_open()) //== wenn datei nicht geöffnet werden kann
    {
        cout << "ICD-Datei konnte nicht geoeffnet werden!\n";
        return;
    }

    string line;

    while (getline(file, line)) //schleife verarbeitet die gesamte datei
    {
        stringstream ss(line); //die zeile bzw line die wir grade bekommen haben bereiten wir vor zum trennen um einzelne spalten zu bekommen
        string column;
        vector<string> columns;

        while (getline(ss, column, ';')) //nimmt immer den nächsten abschnitt bis zum semikolon
        {
            columns.push_back(column); //fügt die gerade ausgelesene spalte hinten an den vector an
        }

        if (columns.size() >= 9) //überprüfen ob mindestens 9 spalten vorhanden sind
        {
            string code = columns[5];       //sechste spalte wird gespeichert in variable code
            string diagnosis = columns[8]; //neunte spalte gespeichert als diagnosetext

            icdCodes[code] = diagnosis; //beide infos werden in map gespeichert
        }
    }

    file.close();

    icdLoaded = true;

    cout << "ICD-Daten erfolgreich eingelesen!\n";
}


// JSON-DATEI EINLESEN  //Krankenhausaufenthalte der patienten stheht hier

void readCaseFile(string filename)
{
    if (!icdLoaded)
    {
        cout << "ICD-Daten werden zuerst geladen.\n";

        readICDData("icdCodes.txt");

        if (!icdLoaded)
            return;
    }

    QFile file(QString::fromStdString(filename));   //Qt klasse mit der dateien geöffnet und bearbeitet werden können normaler c++ string wird in qt string umgewandelt

    if (!file.open(QIODevice::ReadOnly)) //datei wird nur zum lesen geöffnet
    {
        cout << "JSON-Datei konnte nicht geoeffnet werden!\n";
        return;
    }

    QJsonParseError error; // mögliche fehler beim einlesen werden hier gespeichert

    QJsonDocument document =
        QJsonDocument::fromJson(file.readAll(), &error);//readAll liest gesamten inhalt datei; fromJson wandelt inhalt in Json dokument um;error übergibt adresse fehlervariablen damit qt mögliche fehler da speicehrn kann; document speichert fertige JSOn document

    file.close(); //schließen datei

    if (error.error != QJsonParseError::NoError) //wenn fehler ungleich noerror geben wir fehlermeldung und brechen ab
    {
        cout << "Fehler beim Lesen der JSON-Datei!\n";
        return;
    }

    if (!document.isArray()) //überprüfung von json document ob array vorhandne
    {
        cout << "JSON-Datei enthaelt kein Array!\n";
        return;
    }

    QJsonArray array = document.array(); //array aus json dokument geholt und in variable array gespeichert

    patients.clear(); //alle bisher gespeicherten patienten aus vector werden gelöscht damit beim einlesen nichz doppelt speichern

    long stayID = 1;

    for (const QJsonValue &value : array) //for schleife durchläuft alle einträge von json array und wandelt jeden eintrag in json objekt um damit wir auf dessen daten zugreifen können
    {
        QJsonObject object = value.toObject();

        string admissionDate =
            object["admissionDate"].toString().toStdString();

        string dischargeDate =
            object["dischargeDate"].toString().toStdString();

        string diagnosisCode =
            object["mainDiagnosis"].toString().toStdString();

        string firstName =
            object["firstName"].toString().toStdString();

        string lastName =
            object["lastName"].toString().toStdString();

        string birthDate =
            object["birthDate"].toString().toStdString();

        string insuranceNumber =
            object["insuranceNumber"].toString().toStdString();

        string genderString =
            object["gender"].toString().toStdString();


        // GESCHLECHT BESTIMMEN

        Gender gender;

        if (genderString == "M")
            gender = male;
        else if (genderString == "W")
            gender = female;
        else
            gender = nonBinary;


        // DIAGNOSE SUCHEN

        string diagnosisText = "Diagnose nicht gefunden";

        if (icdCodes.find(diagnosisCode) != icdCodes.end())
        {
            diagnosisText = icdCodes[diagnosisCode];
        }


        // PATIENT SUCHEN

        int patientIndex = -1;  //-1 heisst patient noch nicht gefunden

        for (int i = 0; i < patients.size(); i++)
        {
            if (patients[i].getInsuranceNumber() == insuranceNumber)
            {
                patientIndex = i;
                break;
            }
        }


        // NEUEN PATIENTEN ERSTELLEN

        if (patientIndex == -1)
        {
            long patientID = patients.size() + 1;

            Patient newPatient(
                patientID,
                birthDate,
                firstName,
                lastName,
                insuranceNumber,
                gender
            );

            patients.push_back(newPatient);

            patientIndex = patients.size() - 1;
        }


        // AUFENTHALT ERSTELLEN

        long patientID = patients[patientIndex].getPatientID();

        Stay newStay(
            patientID,
            stayID,
            admissionDate,
            dischargeDate,
            diagnosisCode,
            diagnosisText
        );

        patients[patientIndex].addStay(newStay);

        stayID++;
    }

    cout << "Aufenthaltsdaten erfolgreich eingelesen!\n";
}


// PATIENTENLISTE

void patientList()
{
    for (Patient patient : patients)
    {
        cout << patient.showOverview() << endl;
    }
}


// BENUTZERMENUE

void userDialog()
{
    int choice;

    do
    {
        cout << "\n========== " << name << " ==========\n";

        cout << "1: Lese Aufenthaltsdatei\n";
        cout << "2: Lese ICD-Daten\n";
        cout << "3: Liste aller Daten\n";
        cout << "0: Beende das Programm\n";

        cout << "Auswahl: ";

        cin >> choice;

        switch (choice)
        {
            case 1:
                readCaseFile("stays.json");
                break;

            case 2:
                readICDData("icdCodes.txt");
                break;

            case 3:
                patientList();
                break;

            case 0:
                cout << "Programm wird beendet.\n";
                break;

            default:
                cout << "Ungueltige Eingabe!\n";
        }

    } while (choice != 0);
}
};
// MAIN
int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
Hospital hospital("Krankenhaus");

hospital.userDialog();

return 0;
}

