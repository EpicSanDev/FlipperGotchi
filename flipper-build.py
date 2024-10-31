App(
    appid="flipper_gotchi",  # Identifiant unique de l'application
    name="FlipperGotchi",    # Nom affiché dans le menu
    apptype=FlipperAppType.EXTERNAL,
    entry_point="flipper_gotchi_app",  # Point d'entrée (fonction main)
    stack_size=2 * 1024,
    fap_category="Games",
    fap_icon="icon.png",     # Icône 10x10 pixels
    fap_author="VotreNom",   # Votre nom
    fap_version="1.0",
    fap_description="Un Tamagotchi virtuel pour Flipper Zero",
)
