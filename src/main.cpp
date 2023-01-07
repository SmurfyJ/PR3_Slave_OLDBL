#include "Arduino.h"
#include <util/twi.h>

void adc_init();
uint16_t adc_read();
void ERROR();

uint16_t data_out = 0;
uint8_t SLA_W = 2;

int main() {

    init();
    adc_init();

    //Slave Adresse
    TWAR = SLA_W << 1;

    while (1) {

        // Slave aktivieren, ACK fuer eigene Adresse vorbereitet
        TWCR = (1 << TWINT) | (1 << TWEA) | (1 << TWEN);

        // Warten
        while (!(TWCR & (1 << TWINT)));

        // Status ueberpruefen: Eigene Adresse empfangen
        if (TW_STATUS != TW_ST_SLA_ACK) ERROR();

        // Daten senden, ACK erwarten (weil nicht letztes Paket)
        data_out = adc_read();
        TWDR = (data_out >> 8);
        TWCR = (1 << TWINT) | (1 << TWEA) | (1 << TWEN);

        // Warten
        while (!(TWCR & (1 << TWINT)));

        // Status ueberpruefen: Master hat Daten bestaetigt
        if (TW_STATUS != TW_ST_DATA_ACK) ERROR();

        TWDR = (data_out & 0xFF);
        TWCR = (1 << TWINT) | (1 << TWEN);

        // Warten
        while (!(TWCR & (1 << TWINT)));

        // Status ueberpruefen: Master hat Daten bestaetigt
        if (TW_STATUS != TW_ST_DATA_NACK) ERROR();

    }
}

void ERROR() {
    Serial.print("TWCR: ");
    Serial.println(TWCR, BIN);
    Serial.print("TW_STATUS: ");
    Serial.println(TW_STATUS, HEX);
}

void adc_init() {
    //AD-Einstellungen löschen
    ADCSRA = ADCSRB = ADMUX = 0;

    //ADC Aktivieren
    ADCSRA |= (1 << ADEN);

    // Teilungsfaktor einstellen, 128 hier; Abtastrate bis 200kHz möglich, höhere Abtastrate -> kleinerer Teiler
    ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);

    // Referenzspannung einstellen, hier interne 5V
    ADMUX |= (1 << REFS0);

    //Kanal 0 des Multiplexers ist automatisch gesetzt
    //ADMUX |= (0 << MUX0); wäre zb für Kanal 1!!!
}

uint16_t adc_read() {
    // Starte ADC Konvertierung
    ADCSRA |= (1 << ADSC);
    // Warten bis zum Abschluss der Analogumwandlung
    while (ADCSRA & (1 << ADSC));
    return ADC;
}