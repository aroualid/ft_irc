#include "../../incs/core/Client.hpp"
#include <criterion/criterion.h>
#include <criterion/new/assert.h>
#include <string>
#include <set>

// Helper pour instancier un client rapidement
Client* setup_client() {
    return new Client(42, "localhost");
}

void teardown_client(Client* c) {
    delete c;
}

// -----------------------------------------------------------------------------
// Test Suite 1 : Initialisation
// -----------------------------------------------------------------------------

Test(client_suite, initialization) {
    Client c(10, "127.0.0.1");

    cr_assert_eq(c.getFd(), 10, "Le FD devrait être 10");
    cr_assert_str_eq(c.getNickname().c_str(), "", "Le nickname doit être vide au départ");
    cr_assert_str_eq(c.getUsername().c_str(), "", "Le username doit être vide au départ");
    cr_assert_eq(c.isAuthenticated(), false, "Le client ne doit pas être authentifié par défaut");
    cr_assert_eq(c.isRegistered(), false, "Le client ne doit pas être enregistré par défaut");
}

// -----------------------------------------------------------------------------
// Test Suite 2 : Setters et Getters basiques
// -----------------------------------------------------------------------------

Test(client_suite, setters_getters) {
    Client c(1, "host");

    c.setNickname("DarkVador");
    c.setUsername("ani");
    c.setRealname("Anakin Skywalker");

    cr_assert_str_eq(c.getNickname().c_str(), "DarkVador");
    cr_assert_str_eq(c.getUsername().c_str(), "ani");
    cr_assert_str_eq(c.getRealname().c_str(), "Anakin Skywalker");
}

// -----------------------------------------------------------------------------
// Test Suite 3 : Flux d'Enregistrement (Registration State)
// -----------------------------------------------------------------------------

Test(client_suite, registration_flow) {
    Client c(1, "host");

    // 1. Fournir le mot de passe -> Toujours pas enregistré
    c.setPasswordProvided(true);
    cr_assert_eq(c.isAuthenticated(), true, "Le mot de passe est fourni");
    cr_assert_eq(c.isRegistered(), false, "Pas encore enregistré (manque nick et user)");

    // 2. Fournir le Nickname -> Toujours pas enregistré
    c.setNickname("Yoda");
    cr_assert_eq(c.isRegistered(), false, "Pas encore enregistré (manque user)");

    // 3. Fournir le Username -> Doit passer à REGISTERED
    c.setUsername("master");
    cr_assert_eq(c.isRegistered(), true, "Le client devrait être enregistré maintenant");
}

Test(client_suite, registration_partial) {
    Client c(1, "host");

    // Test sans mot de passe
    c.setNickname("Luke");
    c.setUsername("skywalker");
    cr_assert_eq(c.isRegistered(), false, "Ne doit pas être enregistré sans mot de passe");
}

// -----------------------------------------------------------------------------
// Test Suite 4 : Gestion des Channels
// -----------------------------------------------------------------------------

Test(client_suite, channel_management) {
    Client c(1, "host");
    std::string chan1 = "#general";
    std::string chan2 = "#random";

    // Join
    c.joinChannel(chan1);
    cr_assert(c.isInChannel(chan1), "Le client doit être dans #general");
    cr_assert_not(c.isInChannel(chan2), "Le client ne doit pas être dans #random");

    // Check size
    const std::set<std::string>& channels = c.getChannels();
    cr_assert_eq(channels.size(), 1, "Il doit y avoir 1 channel");

    // Join second
    c.joinChannel(chan2);
    cr_assert(c.isInChannel(chan2));
    cr_assert_eq(c.getChannels().size(), 2);

    // Leave
    c.leaveChannel(chan1);
    cr_assert_not(c.isInChannel(chan1), "Le client a quitté #general");
    cr_assert(c.isInChannel(chan2), "Le client est toujours dans #random");
}

// -----------------------------------------------------------------------------
// Test Suite 5 : Génération du Préfixe IRC
// -----------------------------------------------------------------------------

Test(client_suite, prefix_generation) {
    Client c(1, "my.host.com");

    // Cas 1 : Pas de nickname -> retourne le hostname
    cr_assert_str_eq(c.getPrefix().c_str(), "my.host.com", "Sans nick, le prefix est le hostname");

    // Cas 2 : Nickname seulement
    c.setNickname("Guest");
    // Selon votre logique : if (!_username.empty()) ... sinon juste nickname
    cr_assert_str_eq(c.getPrefix().c_str(), "Guest", "Avec nick mais sans user, retourne juste le nick");

    // Cas 3 : Nickname + Username -> Format complet : nick!user@host
    c.setUsername("user123");
    std::string expected = "Guest!user123@my.host.com";
    cr_assert_str_eq(c.getPrefix().c_str(), expected.c_str(), "Format complet attendu");
}
