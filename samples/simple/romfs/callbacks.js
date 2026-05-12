const daysOfWeek = ["Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"]
const monthLabels = ["January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"]
const numOfWeekdayPanels = 42
const currentMonth = 12
const currentWeek = 51
const currentDay = 20
const currentWeekday = 0
const todayYear = 2024
const todayMonth = 12
const todayDay = 20
const currentTopics = []
const currentTimespanType = 1
const firstDayOfTheCurrentMonth = 0
const numberOfDaysInTheCurrentMonth = 0
const numberOfDaysInThePreviousMonth = 0
const inactiveMonthdayTextColor = 0xFF313131
const activeMonthdayTextColor = 0xFFECEDEE
const sideCalendarMonthdayBackgroundColor = 0xFF0A0A0A
const mainCalendarMonthdayBackgroundColor = 0xFF0E0F10
const todayMonthdayBackgroundColor = 0xFF06193A

function InitializeCalendar() {
  RefreshCurrentCalendarDate()
  RefreshComponentsWithCurrentlySelectedDate()
  ChangeCurrentlyViewedTimespan(null, 1)
}

function MapCurrentWeekdayToMondayBasedWeek(weekday) {
  return (weekday + 6) % 7
}

function GetWeekNumber(year, month, day) {
  const currentDayOfTheWeek = MapCurrentWeekdayToMondayBasedWeek(getWeekday(year, month, day))
  const nextWeekYear = new Date(year, month - 1, day + (7 - currentDayOfTheWeek)).getFullYear()
  if (nextWeekYear != year) {
    return 1
  }
  const weekday = getWeekday(year, 1, 1)
  const mappedWeekday = MapCurrentWeekdayToMondayBasedWeek(weekday)
  const startOfYearFirstWeek = new Date(year, 0, 1 - mappedWeekday)
  const currentDate = new Date(year, month - 1, day)
  const timeDifference = currentDate - startOfYearFirstWeek
  const dayOfYear = Math.floor(timeDifference / (1000 * 60 * 60 * 24))
  const weekNumber = Math.ceil((dayOfYear + 1) / 7)
  return weekNumber
}

function RefreshCurrentCalendarDate() {
  const currentDate = new Date()
  currentYear = todayYear = currentDate.getFullYear()
  currentMonth = todayMonth = currentDate.getMonth() + 1
  currentDay = todayDay = currentDate.getDate()
  currentWeek = GetWeekNumber(currentYear, currentMonth, currentDay)
  currentWeekday = MapCurrentWeekdayToMondayBasedWeek(currentDate.getDay())
  firstDayOfTheCurrentMonth = getWeekday(currentYear, currentMonth, 0)
  numberOfDaysInTheCurrentMonth = getNumberOfDaysInTheMonth(currentYear, currentMonth)
  numberOfDaysInThePreviousMonth = getDaysInPreviousMonth(currentYear, currentMonth)
}

function RefreshComponentsWithCurrentlySelectedDate() {
  firstDayOfTheCurrentMonth = getWeekday(currentYear, currentMonth, 0)
  numberOfDaysInTheCurrentMonth = getNumberOfDaysInTheMonth(currentYear, currentMonth)
  numberOfDaysInThePreviousMonth = getDaysInPreviousMonth(currentYear, currentMonth)
  MakeSureCurrentDayIsInTheCurrentMonthSpan()
  UpdateCurrentlyViewedEvents()
  const calendar = GetElementById("sidePanel").GetElementById("sidePanelCalendar")
  const monthLabel = calendar.GetElementById("currentMonthLabel")
  monthLabel.text = monthLabels[currentMonth - 1] + " " + currentYear
  const calendarGrid = calendar.GetElementById("calendarSidePanelGrid")
  UpdateMainCalendarMonthLabel()
  UpdateMainCalendarWeekdayLabels()
}

function ChangeMonthCallback(caller, direction) {
  ChangeMonth(direction)
  RefreshComponentsWithCurrentlySelectedDate()
}

function ChangeMonth(direction) {
  currentMonth += Number(direction)
  if (currentMonth <= 0) {
    currentYear -= 1
    currentMonth = 12
  } else if (currentMonth > 12) {
    currentYear += 1
    currentMonth = 1
  }
  firstDayOfTheCurrentMonth = getWeekday(currentYear, currentMonth, 0)
  numberOfDaysInTheCurrentMonth = getNumberOfDaysInTheMonth(currentYear, currentMonth)
  numberOfDaysInThePreviousMonth = getDaysInPreviousMonth(currentYear, currentMonth)
}

function getNumberOfDaysInTheMonth(year, month) {
  const date = new Date(year, month, 0)
  return date.getDate()
}

function getWeekday(year, month, day) {
  const date = new Date(year, month - 1, day)
  return date.getDay()
}

function changeWeekdayPanelVisibility(calendar, panelIndex, visibility, backgroundColor) {
  const weekdayPanel = calendar.GetElementById(panelIndex)
  weekdayPanel.backgroundColor = backgroundColor
  weekdayPanel.textColor = visibility ? activeMonthdayTextColor : inactiveMonthdayTextColor
}

function updateWeekdayPanelMonthDay(calendar, panelIndex, monthDay) {
  const weekdayLabel = calendar.GetElementById(panelIndex)
  weekdayLabel.text = monthDay
}

function UpdateWeekdayIfIsToday(calendar, panelIndex, monthDay, year, month) {
  if (year !== todayYear || month !== todayMonth || monthDay !== todayDay) {
    return
  }
  const weekdayLabel = calendar.GetElementById(panelIndex)
  weekdayLabel.backgroundColor = todayMonthdayBackgroundColor
}

function getDaysInPreviousMonth(currentYear, currentMonth) {
  const previousYear = currentYear;
  const previousMonth = currentMonth - 1;
  if (previousMonth <= 0) {
    previousYear -= 1
    previousMonth = 12
  }
  return getNumberOfDaysInTheMonth(previousYear, previousMonth)
}

function ChangeCurrentlyViewedDay(caller) {
  currentDay = Number(caller.name) - firstDayOfTheCurrentMonth + 1
  RefreshComponentsWithCurrentlySelectedDate()
}

function MakeSureCurrentDayIsInTheCurrentMonthSpan() {
  const calendar = GetElementById("sidePanel").GetElementById("sidePanelCalendar").GetElementById("calendarSidePanelGrid")
  if (currentDay <= 0) {
    ChangeMonth(-1)
    const currentDayLabelName = numberOfDaysInTheCurrentMonth + currentDay + firstDayOfTheCurrentMonth - 1
    currentDay = numberOfDaysInTheCurrentMonth + currentDay
    calendar.SetCurrentlySelectedItem(currentDayLabelName)
  } else if (currentDay > numberOfDaysInTheCurrentMonth) {
    ChangeMonth(1)
    const currentDayLabelName = currentDay - numberOfDaysInThePreviousMonth + firstDayOfTheCurrentMonth - 1
    currentDay = currentDay - numberOfDaysInThePreviousMonth
    calendar.SetCurrentlySelectedItem(currentDayLabelName)
  } else {
    const currentDayLabelName = currentDay + firstDayOfTheCurrentMonth - 1
    calendar.SetCurrentlySelectedItem(currentDayLabelName)
  }
  currentWeek = GetWeekNumber(currentYear, currentMonth, currentDay)
  currentWeekday = getWeekday(currentYear, currentMonth, currentDay)
}

function UpdateMainCalendarMonthLabel() {
  const mainCalendarMonthLabel = GetElementById("currentMonth")
  mainCalendarMonthLabel.text = monthLabels[currentMonth - 1] + " " + currentYear
}

function UpdateMainCalendarWeekdayLabels() {
  const calendarLabels = ["mondayCalendarColumn", "tuesdayCalendarColumn", "wednesdayCalendarColumn", "thursdayCalendarColumn", "fridayCalendarColumn", "saturdayCalendarColumn", "sundayCalendarColumn"]
  const monthDays = GetElementById("mainCalendar").GetElementById("calendar").GetElementById("weekMonthDayLabels")
  const weekdayMap = [6, 0, 1, 2, 3, 4, 5]
  const mappedWeekday = weekdayMap[currentWeekday]
  const numberOfDaysInTheWeek = 7
  for (var index = 0; index < numberOfDaysInTheWeek; ++index) {
    const currentDayIndex = currentDay + index - mappedWeekday
    if (currentDayIndex <= 0) {
      currentDayIndex += numberOfDaysInThePreviousMonth
    }
    if (currentDayIndex > numberOfDaysInTheCurrentMonth) {
      currentDayIndex -= numberOfDaysInTheCurrentMonth
    }
    const label = monthDays.GetElementById(calendarLabels[index])
    label.text = currentDayIndex
  }
  const todayLabel = GetElementById("todayLabel")
  todayLabel.text = daysOfWeek[mappedWeekday] + " " + currentDay
}
const events = {}
const event1 = {
  name: "Tennis",
  topic: "Personal",
  notes: "Be ready and well rested!",
  year: 2025,
  month: 7,
  week: 26,
  day: 1,
  time: 540,
  duration: 45
}
const event2 = {
  name: "Basketball",
  topic: "Personal",
  notes: "Invite your tall friends please",
  year: 2025,
  month: 7,
  week: 26,
  day: 2,
  time: 540,
  duration: 15
}
const event3 = {
  name: "Football",
  topic: "Personal",
  notes: "Bring your athletic shoes",
  year: 2025,
  month: 7,
  week: 26,
  day: 3,
  time: 540,
  duration: 60
}
const event4 = {
  name: "Volleyball",
  topic: "Personal",
  notes: "We are meeting at the beach",
  year: 2025,
  month: 7,
  week: 26,
  day: 4,
  time: 720,
  duration: 45
}
const events = [event1, event2, event3, event4]
const currentlyViewedEvents = []
const availableTopics = ["Personal", "Work"]
const topicsColoringMap = {}
topicsColoringMap["Personal"] = {
  borderColor: 0xFF008FFF,
  backgroundColor: 0xFF06193A,
  foregroundColor: 0xFF008FFF,
  descriptionColor: 0xFF005FCF
}
topicsColoringMap["Work"] = {
  borderColor: 0xFF00CFB7,
  backgroundColor: 0xFF00231B,
  foregroundColor: 0xFF00CFB7,
  descriptionColor: 0xFF009F87
}
const dayTimespan = 0
const weekTimespan = 1
const monthTimespan = 2
const yearTimespan = 3
const allTimespan = 4
const eventItemWidth = 75
const eventItemHeight = 60
const eventItemsXOffset = 54
const eventItemsYOffset = 0
const basicEventTimespan = 30
const startingRowTime = 0
const lastNonMonthViewScrollValue = 0
const lastMonthViewScrollValue = 0
const instantiatedEvents = []

function ChangeCurrentlyViewedWeek(caller, direction) {
  const numberOfDaysInTheWeek = 7
  currentDay += numberOfDaysInTheWeek * direction
  const currentDayLabelName = firstDayOfTheCurrentMonth + currentDay - 1
  const calendar = GetElementById("sidePanel").GetElementById("sidePanelCalendar")
  calendar.SetCurrentlySelectedItem(currentDayLabelName)
  RefreshComponentsWithCurrentlySelectedDate()
}

function ChangeCurrentlyViewedTimespan(caller, timespan) {
  const lastTimespanType = currentTimespanType
  currentTimespanType = Number(timespan)
  UpdateCurrentlyViewedEvents()
  ChangeCurrentlyViewedTimespanGrid(lastTimespanType)
}

function ChangeCurrentlyViewedTimespanGrid(lastTimespanType) {
  const mainCalendarGrid = GetElementById("mainCalendar").GetElementById("calendar").GetElementById("mainCalendarPanels")
  const currentDayEventsView = mainCalendarGrid.GetElementById("currentDayEventsGrid")
  const currentWeekEventsView = mainCalendarGrid.GetElementById("currentWeekEventsGrid")
  const mainCalendarTimeLabels = mainCalendarGrid.GetElementById("mainCalendarTimeLabels")
  const weekdayLabels = GetElementById("weekdayLabels")
  const weekMonthDayLabels = GetElementById("weekMonthDayLabels")
  const monthLabel = GetElementById("mainCalendar").GetElementById("topPanel").GetElementById("currentMonth")
  const todayLabel = GetElementById("todayLabel")
  const mainCalendarScrollPanel = GetElementById("mainCalendarPanels")
  const currentTimeLineLabel = mainCalendarGrid.GetElementById("currentTimeLineLabel")
  const currentTimeLine = mainCalendarGrid.GetElementById("currentTimeLine")
  const currentDayLine = mainCalendarGrid.GetElementById("currentDayLine")
  const currentDayLineEllipse = mainCalendarGrid.GetElementById("currentDayLineEllipse")
  if (lastTimespanType == dayTimespan || lastTimespanType == weekTimespan) {
    lastNonMonthViewScrollValue = mainCalendarScrollPanel.scroll
  } else if (lastTimespanType == monthTimespan) {
    lastMonthViewScrollValue = mainCalendarScrollPanel.scroll
  }
  switch (currentTimespanType) {
    case dayTimespan:
      mainCalendarGrid.visibility = true
      currentDayEventsView.visibility = true
      currentWeekEventsView.visibility = false
      mainCalendarTimeLabels.visibility = true
      weekdayLabels.visibility = false
      weekMonthDayLabels.visibility = false
      todayLabel.visibility = true
      currentTimeLineLabel.visibility = true
      currentTimeLine.visibility = true
      currentDayLine.visibility = false
      currentDayLineEllipse.visibility = false
      mainCalendarScrollPanel.Refresh()
      mainCalendarScrollPanel.scroll = lastNonMonthViewScrollValue
      break
    case weekTimespan:
      mainCalendarGrid.visibility = true
      currentDayEventsView.visibility = false
      currentWeekEventsView.visibility = true
      mainCalendarTimeLabels.visibility = true
      weekdayLabels.visibility = true
      weekMonthDayLabels.visibility = true
      todayLabel.visibility = false
      monthLabel.visibility = true
      currentTimeLineLabel.visibility = true
      currentTimeLine.visibility = true
      currentDayLine.visibility = true
      currentDayLineEllipse.visibility = true
      mainCalendarScrollPanel.Refresh()
      mainCalendarScrollPanel.scroll = lastNonMonthViewScrollValue
      break
    case monthTimespan:
      mainCalendarGrid.visibility = true
      currentDayEventsView.visibility = false
      currentWeekEventsView.visibility = false
      mainCalendarTimeLabels.visibility = false
      weekdayLabels.visibility = true
      weekMonthDayLabels.visibility = false
      todayLabel.visibility = false
      monthLabel.visibility = true
      currentTimeLineLabel.visibility = false
      currentTimeLine.visibility = false
      currentDayLine.visibility = false
      currentDayLineEllipse.visibility = false
      mainCalendarScrollPanel.Refresh()
      mainCalendarScrollPanel.scroll = lastMonthViewScrollValue
      break
    case yearTimespan:
      mainCalendarGrid.visibility = true
      currentDayEventsView.visibility = false
      currentWeekEventsView.visibility = false
      mainCalendarTimeLabels.visibility = false
      weekdayLabels.visibility = false
      weekMonthDayLabels.visibility = false
      todayLabel.visibility = false
      monthLabel.visibility = false
      currentTimeLineLabel.visibility = false
      currentTimeLine.visibility = false
      currentDayLine.visibility = false
      currentDayLineEllipse.visibility = false
      mainCalendarScrollPanel.Refresh()
      break
    case allTimespan:
      mainCalendarGrid.visibility = false
      currentDayEventsView.visibility = false
      currentWeekEventsView.visibility = false
      mainCalendarTimeLabels.visibility = false
      weekdayLabels.visibility = false
      weekMonthDayLabels.visibility = false
      todayLabel.visibility = false
      monthLabel.visibility = false
      currentTimeLineLabel.visibility = false
      currentTimeLine.visibility = false
      currentDayLine.visibility = false
      currentDayLineEllipse.visibility = false
      mainCalendarScrollPanel.Refresh()
      break
    default:
      Print("Invalid timespan");
  }
}

function UpdateCurrentlyViewedEvents() {
  if (currentTimespanType === allTimespan) {
    const searchTextInput = GetElementById("mainCalendar").GetElementById("topPanel").GetElementById("search")
    UpdateSearchedEvents(searchTextInput.text)
    return
  }
  UpdateCurrentlyViewedCalendarEvents()
}

function UpdateCurrentlyViewedCalendarEvents() {
  removeAllCurrentlyInstantiatedEvents()
  currentlyViewedEvents = applyTimeAndTopicFiltersOnEvents()
  SortEventsByTimeIfCurrentViewIsMonthView()
  instantiatePrefabsForAllCurrentlyViewedEvents()
}

function removeAllCurrentlyInstantiatedEvents() {
  const mainCalendarPanels = GetElementById("mainCalendarPanels")
  for (var i = 0; i < instantiatedEvents.length; i++) {
    const event = instantiatedEvents[i]
    if (mainCalendarPanels.HasElement(event)) {
      mainCalendarPanels.RemoveElement(event)
    }
  }
}

function applyTimeAndTopicFiltersOnEvents() {
  const filteredEvents = events
  switch (currentTimespanType) {
    case dayTimespan:
      filteredEvents = filteredEvents.filter(function(event) {
        return event.day === currentDay
      })
      break
    case weekTimespan:
      filteredEvents = filteredEvents.filter(function(event) {
        return event.week === currentWeek
      })
      break
    case monthTimespan:
      filteredEvents = filteredEvents.filter(function(event) {
        return event.month === currentMonth
      })
      break
    case yearTimespan:
      filteredEvents = filteredEvents.filter(function(event) {
        return event.year === currentYear
      })
      break
    default:
  }
  filteredEvents = filteredEvents.filter(function(event) {
    return currentTopics.indexOf(event.topic) !== -1
  })
  return filteredEvents
}

function SortEventsByTimeIfCurrentViewIsMonthView() {
  if (currentTimespanType != monthTimespan) {
    return;
  }
  currentlyViewedEvents.sort(function(first, second) {
    return first.time - second.time;
  });
}

function GetDayIndex(day) {
  if (day <= 0) {
    return numberOfDaysInThePreviousMonth + day
  }
  if (day > numberOfDaysInTheCurrentMonth) {
    return day - numberOfDaysInTheCurrentMonth
  }
  return day
}

function instantiatePrefabsForAllCurrentlyViewedEvents() {
  if (currentTimespanType === yearTimespan || currentTimespanType === allTimespan) {
    return
  }
  const eventPrefab = GetPrefabById("eventItem")
  const mainCalendarPanels = GetElementById("mainCalendarPanels")
  monthViewEventsBuckets = {}
  for (var i = 0; i < currentlyViewedEvents.length; i++) {
    const event = currentlyViewedEvents[i]
    const clonedPrefab = eventPrefab.Clone()
    clonedPrefab.backgroundColor = topicsColoringMap[event.topic].backgroundColor
    clonedPrefab.borderColor = topicsColoringMap[event.topic].borderColor
    const eventLabel = clonedPrefab.GetElementById("name")
    eventLabel.text = event.name
    eventLabel.textColor = topicsColoringMap[event.topic].foregroundColor
    const eventDescription = clonedPrefab.GetElementById("description")
    eventDescription.text = event.notes
    eventDescription.visibility = currentTimespanType == dayTimespan
    eventDescription.textColor = topicsColoringMap[event.topic].descriptionColor
    AdjustEventPositionAndSizeBasedOnTheCurrentTimespan(monthViewEventsBuckets, clonedPrefab, event)
    instantiatedEvents.push(clonedPrefab.name)
    mainCalendarPanels.AddElement(clonedPrefab)
  }
}

function AdjustEventPositionAndSizeBasedOnTheCurrentTimespan(monthViewEventsBuckets, instantiatedEvent, sourceEvent) {
  const weekdayMap = [6, 0, 1, 2, 3, 4, 5]
  const eventLabel = instantiatedEvent.GetElementById("name")
  switch (currentTimespanType) {
    case dayTimespan:
      instantiatedEvent.x = eventItemsXOffset
      const row = (sourceEvent.time - startingRowTime) / basicEventTimespan * eventItemHeight
      const mappedWeekday = weekdayMap[getWeekday(sourceEvent.year, sourceEvent.month, sourceEvent.day)]
      instantiatedEvent.y = eventItemsYOffset + row
      instantiatedEvent.width = eventItemWidth * 7
      const height = sourceEvent.duration / basicEventTimespan * eventItemHeight
      instantiatedEvent.height = height
      eventLabel.width = instantiatedEvent.width
      break
    case weekTimespan:
      const mappedWeekday = weekdayMap[getWeekday(sourceEvent.year, sourceEvent.month, sourceEvent.day)]
      const column = mappedWeekday * eventItemWidth
      instantiatedEvent.x = eventItemsXOffset + column
      const row = (sourceEvent.time - startingRowTime) / basicEventTimespan * eventItemHeight
      instantiatedEvent.y = eventItemsYOffset + row
      const height = sourceEvent.duration / basicEventTimespan * eventItemHeight
      instantiatedEvent.height = height
      eventLabel.width = instantiatedEvent.width
      break
    case monthTimespan:
      const mappedWeekday = weekdayMap[getWeekday(sourceEvent.year, sourceEvent.month, sourceEvent.day)]
      const columnOffset = mappedWeekday * eventItemWidth
      instantiatedEvent.x = eventItemsXOffset + columnOffset
      const dayBucket = firstDayOfTheCurrentMonth + sourceEvent.day - 1
      const row = Math.floor(dayBucket / 7) * monthViewPanelHeight
      if (!monthViewEventsBuckets.hasOwnProperty(dayBucket)) {
        monthViewEventsBuckets[dayBucket] = 0
      }
      instantiatedEvent.y = row + monthViewPanelEventVerticalOffset + monthViewEventsBuckets[dayBucket] * (monthViewEventHeight + monthViewEventVerticalOffset)
      monthViewEventsBuckets[dayBucket] += 1
      instantiatedEvent.width = monthViewEventWidth
      instantiatedEvent.height = monthViewEventHeight
      eventLabel.height = monthViewEventHeight
      break
    default:
      Print("Invalid timespan");
  }
}

function AddNewCalendar(caller) {
  const prefabName = "calendarItem"
  const prefab = GetPrefabById(prefabName)
  const clone = prefab.Clone()
  const calendarsList = GetElementById("sidePanel").GetElementById("sidePanelCalendar").GetElementById("calendarsList")
  calendarsList.AddElement(clone)
}

function AddOrRemoveTopic(caller) {
  const parent = GetElementById(caller.parentName)
  const calendarTopicLabel = parent.GetElementByIndex(1)
  const topicName = calendarTopicLabel.text
  if (caller.switchState) {
    currentTopics.push(topicName)
    Print("Adding calendar topic " + topicName)
    UpdateCurrentlyViewedEvents()
  } else {
    for (var i = 0; i < currentTopics.length; i++) {
      if (currentTopics[i] === topicName) {
        currentTopics.splice(i, 1);
        UpdateCurrentlyViewedEvents()
        Print("Removing calendar topic " + topicName)
        break
      }
    }
  }
}

function UpdateCurrentTime() {
  const currentTimeLabel = GetElementById("sidePanel").GetElementById("currentTime")
  currentTimeLabel.text = GetCurrentTimeFormatted()
}
const abbreviatedMonthNames = ["Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"]

function GetCurrentTimeFormatted() {
  const now = new Date();
  const day = now.getDate();
  const weekday = daysOfWeek[MapCurrentWeekdayToMondayBasedWeek(now.getDay())];
  const month = abbreviatedMonthNames[now.getMonth()];
  const hours = now.getHours();
  const minutes = now.getMinutes();
  const ampm = hours >= 12 ? "PM" : "AM";
  hours = hours % 12;
  hours = hours ? hours : 12;
  minutes = minutes < 10 ? "0" + minutes : minutes;
  const formattedTime = weekday + " " + day + " " + month + " " + hours + ":" + minutes + " " + ampm;
  return formattedTime;
}

function UpdatePositionOfCurrentTimeLine() {
  const currentTimeLineLabel = GetElementById("mainCalendar").GetElementById("calendar").GetElementById("mainCalendarPanels").GetElementById("currentTimeLineLabel")
  const currentTimeLine = GetElementById("mainCalendar").GetElementById("calendar").GetElementById("mainCalendarPanels").GetElementById("currentTimeLine")
  const currentDayLine = GetElementById("mainCalendar").GetElementById("calendar").GetElementById("mainCalendarPanels").GetElementById("currentDayLine")
  const currentDayLineEllipse = GetElementById("mainCalendar").GetElementById("calendar").GetElementById("mainCalendarPanels").GetElementById("currentDayLineEllipse")
  const now = new Date();
  const hours = now.getHours();
  const minutes = now.getMinutes();
  minutes = minutes < 10 ? "0" + minutes : minutes;
  currentTimeLineLabel.text = hours + ":" + minutes
  const newLineY = (hours * 60.0 + Number(minutes)) / 30.0 * eventItemHeight
  currentTimeLine.y = newLineY
  currentTimeLineLabel.y = newLineY - currentTimeLineLabel.height / 2.0
  const weekday = MapCurrentWeekdayToMondayBasedWeek(now.getDay());
  const newDayLineX = eventItemsXOffset + eventItemWidth * weekday
  currentDayLine.x = newDayLineX
  currentDayLine.y = newLineY
  currentDayLineEllipse.x = newDayLineX
  currentDayLineEllipse.y = newLineY - currentDayLineEllipse.height / 2.0
}
const currentDayFrameDotXOffset = 24
const currentDayFrameDotYOffset = 6

const eventsSearchItemHeight = 39
const eventsSearchItemVerticalGap = 10
const currentlyViewedSearchedEvents = []

function ChangeCurrentlyViewedTimespanToAllEvents() {
  ChangeCurrentlyViewedTimespan(null, allTimespan)
  UpdateSearchedEvents("")
}

function SearchEventsTextInputCallback(caller) {
  UpdateSearchedEvents(caller.text)
}

function UpdateSearchedEvents(searchedPhrase) {
  filteredEvents = ApplyFilterForSearchedEvents(searchedPhrase)
  UpdateCurrentlyVisibleSearchedEvents(filteredEvents)
}

function ApplyFilterForSearchedEvents(searchedPhrase) {
  const filteredEvents = events.filter(function(event) {
    return currentTopics.indexOf(event.topic) !== -1
  })
  filteredEvents = filteredEvents.filter(function(event) {
    return event.name.indexOf(searchedPhrase) !== -1
  })
  return filteredEvents
}

function UpdateCurrentlyVisibleSearchedEvents(filteredEvents) {
  const searchedEventPrefab = GetPrefabById("eventSearchItem")
  for (var i = 0; i < filteredEvents.length; i++) {
    const event = filteredEvents[i]
    const clonedPrefab = searchedEventPrefab.Clone()
    clonedPrefab.y = i * (eventsSearchItemHeight + eventsSearchItemVerticalGap)
    const topicDot = clonedPrefab.GetElementById("topicDot")
    topicDot.backgroundColor = topicsColoringMap[event.topic].foregroundColor
    topicDot.borderColor = topicsColoringMap[event.topic].foregroundColor
    const dateLabel = clonedPrefab.GetElementById("date")
    dateLabel.text = GetFormattedDate(event.year, event.month, event.day)
    const timeLabel = clonedPrefab.GetElementById("time")
    timeLabel.text = GetFormattedTime(event.time)
    const nameLabel = clonedPrefab.GetElementById("name")
    nameLabel.text = event.name
    currentlyViewedSearchedEvents.push(clonedPrefab.name)
  }
}

function GetFormattedDate(year, month, day) {
  const date = new Date(year, month - 1, day)
  return date.toDateString()
}

function GetFormattedTime(timeInMinutes) {
  const hours = parseInt(timeInMinutes / 60);
  hours = hours < 10 ? "0" + hours : hours
  const minutes = timeInMinutes % 60;
  minutes = minutes < 10 ? "0" + minutes : minutes;
  return hours + ":" + minutes
}
