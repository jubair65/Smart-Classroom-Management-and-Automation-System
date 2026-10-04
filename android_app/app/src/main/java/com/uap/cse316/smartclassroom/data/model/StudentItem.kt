package com.uap.cse316.smartclassroom.data.model

import com.google.firebase.database.IgnoreExtraProperties

@IgnoreExtraProperties
data class StudentItem(
    val name: String = "",
    val enterTime: String = "",
    val date: String = "",
    val tagId: String = ""
)
