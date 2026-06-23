-- Crearea bazei de date
CREATE DATABASE IF NOT EXISTS retea_socializare;
USE retea_socializare;

-- 1. Tabelul principal pentru utilizatori
CREATE TABLE utilizatori (
    id INT AUTO_INCREMENT PRIMARY KEY,
    username VARCHAR(50) NOT NULL UNIQUE,
    nume_real VARCHAR(100) NOT NULL,
    email VARCHAR(100) NOT NULL UNIQUE,
    
    -- Intotdeauna stocam parolele hash-uite (criptate), niciodata in clar!
    parola_hash VARCHAR(255) NOT NULL,
    
    -- Caching pentru contoare (pentru a se incarca profilul rapid)
    nr_urmaritori INT DEFAULT 0,
    nr_urmariti INT DEFAULT 0,
    
    data_inregistrare TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- 2. Tabelul pentru relatiile de urmarire (Graful)
-- Aici se construiesc "muchiile" grafului tau pentru algoritmii din C
CREATE TABLE relatii_urmarire (
    id_urmaritor INT NOT NULL,
    id_urmarit INT NOT NULL,
    data_urmarire TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    
    -- Cheia primara compusa ne asigura ca un utilizator nu poate urmari aceeasi persoana de 2 ori
    PRIMARY KEY (id_urmaritor, id_urmarit),
    
    -- Conectam id-urile cu tabelul de utilizatori
    -- Daca un utilizator isi sterge contul (CASCADE), se sterg automat si relatiile lui
    FOREIGN KEY (id_urmaritor) REFERENCES utilizatori(id) ON DELETE CASCADE,
    FOREIGN KEY (id_urmarit) REFERENCES utilizatori(id) ON DELETE CASCADE
);

-- Indexuri pentru a face cautarile super rapide cand serverul C cere date
CREATE INDEX idx_username ON utilizatori(username);
CREATE INDEX idx_urmarit ON relatii_urmarire(id_urmarit);