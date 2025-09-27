def hand_value(hand: list[str]) -> int:
    value = 0
    n_aces = 0
    for card in hand:
        if card in ['J', 'Q', 'K']:
            value += 10
        elif card == 'A':
            value += 11
            n_aces += 1
        else:
            value += int(card)
        if value > 21 and n_aces:
            value -= 10
            n_aces -= 1

    if value == 21 and len(hand) == 2:
        return 22
    return value

bank_cards = input().split()
player_cards = input().split()

bank_value = hand_value(bank_cards)
player_value = hand_value(player_cards)

if player_value == 22 and bank_value != 22:
    print("Blackjack!")

elif player_value == bank_value and player_value <= 22:
    print("Draw")

elif player_value <= 21 and (player_value > bank_value or bank_value > 21):
    print("Player")

else:
    print("Bank")
