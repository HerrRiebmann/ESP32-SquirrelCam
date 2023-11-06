void LoadBotUsers() {
  settings.begin("BotUser", true);
  size_t len = settings.getBytesLength("user");
  settings.getBytes("user", users, len);
  settings.end();

  for (uint8_t i = 0; i < MAX_BOT_USER; i++) {
    if (users[i].chatId == 0 || users[i].userType == Empty)
      continue;
    PrintMessage("User ");
    PrintMessage(String(i));
    PrintMessage(": ");
    PrintMessage(String(users[i].chatId));
    PrintMessage(" - ");
    switch (users[i].userType) {
      case Admin:
        PrintMessageLn("Admin");
        break;
      case Subscriber:
        PrintMessageLn("Subscriber");
        break;
      case Undefined:
        PrintMessageLn("Undefined");
        break;
      default:
        PrintMessageLn("Empty");
    }
  }
}

void StoreBotUsers() {
  settings.begin("BotUser", false);
  settings.putBytes("user", &users, sizeof(users));
  settings.end();
}

bool AddBotUser(long chatId, UserType userType) {
  for (uint8_t i = 0; i < MAX_BOT_USER; i++)
    if (users[i].chatId == 0 && users[i].userType == Empty) {
      users[i].chatId = chatId;
      users[i].userType = userType;
      StoreBotUsers();
      PrintMessageLn("ChatId " + String(chatId) + " added");
      return true;
    }
  return false;
}

bool RemoveBotUser(long chatId) {
  for (uint8_t i = 0; i < MAX_BOT_USER; i++)
    if (users[i].chatId == chatId) {
      users[i].chatId = 0;
      users[i].userType = Empty;
      PrintMessageLn("ChatId " + String(chatId) + " removed");
      return true;
    }
  return false;
}
void ChangeBotUser(long chatId, UserType userType) {
  for (uint8_t i = 0; i < MAX_BOT_USER; i++)
    if (users[i].chatId == chatId) {
      users[i].userType = userType;
      PrintMessageLn("ChatId " + String(chatId) + " updated");
    }
}
bool BotUserExists(long chatId) {
  for (uint8_t i = 0; i < MAX_BOT_USER; i++)
    if (users[i].chatId == chatId)
      return true;
  return false;
}

void ProcessBotUser(long chatId, int user_type) {
  //Limit to max enum value
  if (user_type > 3)
    user_type = 3;
  UserType userType = static_cast<UserType>(user_type);

  if (chatId > 0 && userType == Empty) {
    if (BotUserExists(chatId))
      RemoveBotUser(chatId);
  }
  else {
    if (!BotUserExists(chatId))
      AddBotUser(chatId, userType);
    else
      ChangeBotUser(chatId, userType);
  }
  StoreBotUsers();
}

bool BotUserIsAdmin(String chatId) {
  return BotUserIsAdmin(chatId.toInt());
}
bool BotUserIsAdmin(long chatId) {
  for (uint8_t i = 0; i < MAX_BOT_USER; i++)
    if (users[i].chatId == chatId)
      return users[i].userType == Admin;
  return false;
}
bool BotUserIsReceipient(String chatId) {
  return BotUserIsReceipient(chatId.toInt());
}
bool BotUserIsReceipient(long chatId) {
  for (uint8_t i = 0; i < MAX_BOT_USER; i++)
    if (users[i].chatId == chatId)
      return users[i].userType == Admin || users[i].userType == Subscriber;
  return false;
}

void CreateAdminAccount() {
  users[0].chatId = String(MY_CHAT_ID).toInt();
  users[0].userType = Admin;
}

void TestBotUsers() {
  LoadBotUsers();
  uint8_t userCount = 0;
  for (uint8_t i = 0; i < MAX_BOT_USER; i++)
    if (users[i].chatId != 0 && users[i].userType != Empty)
      userCount++;

  if (userCount == 0) {
    users[0].chatId = String(MY_CHAT_ID).toInt();
    users[0].userType = Admin;
    userCount = 1;
  } else if (userCount > 0 && userCount < MAX_BOT_USER) {
    users[userCount].chatId = 123456780 + userCount;
    users[userCount].userType = userCount % 2 == 0 ? Subscriber : Undefined;
    userCount++;
  } else {
    for (uint8_t i = 0; i < MAX_BOT_USER; i++) {
      users[i].chatId = 0;
      users[i].userType = Empty;
    }
  }
  StoreBotUsers();
}
