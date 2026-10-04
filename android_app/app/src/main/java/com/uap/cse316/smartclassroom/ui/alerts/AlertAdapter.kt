package com.uap.cse316.smartclassroom.ui.alerts

import android.view.LayoutInflater
import android.view.ViewGroup
import androidx.recyclerview.widget.RecyclerView
import com.uap.cse316.smartclassroom.data.model.AlertItem
import com.uap.cse316.smartclassroom.databinding.ItemAlertBinding
import java.text.SimpleDateFormat
import java.util.Date
import java.util.Locale

class AlertAdapter(private val alertsList: MutableList<AlertItem> = mutableListOf()) :
    RecyclerView.Adapter<AlertAdapter.AlertViewHolder>() {

    fun setAlerts(newAlerts: List<AlertItem>) {
        alertsList.clear()
        // Display latest alerts first (reverse chronological)
        alertsList.addAll(newAlerts.reversed())
        notifyDataSetChanged()
    }

    override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): AlertViewHolder {
        val binding = ItemAlertBinding.inflate(LayoutInflater.from(parent.context), parent, false)
        return AlertViewHolder(binding)
    }

    override fun onBindViewHolder(holder: AlertViewHolder, position: Int) {
        holder.bind(alertsList[position])
    }

    override fun getItemCount(): Int = alertsList.size

    inner class AlertViewHolder(private val binding: ItemAlertBinding) :
        RecyclerView.ViewHolder(binding.root) {

        fun bind(item: AlertItem) {
            val cleanTitle = when (item.type.uppercase()) {
                "UNKNOWN_PERSON" -> "Unverified Entry"
                "UNKNOWN" -> "Unregistered Card"
                else -> item.type.replace('_', ' ').lowercase().replaceFirstChar { it.uppercase() }
            }
            binding.tvAlertTitle.text = cleanTitle
            binding.tvAlertMessage.text = item.message

            // Format date and time
            val dateStr = if (item.date.isNotBlank()) {
                item.date
            } else if (item.timestamp > 0) {
                SimpleDateFormat("dd MMM yyyy", Locale.getDefault()).format(Date(item.timestamp))
            } else {
                ""
            }

            val timeStr = if (item.time.isNotBlank()) item.time else ""

            binding.tvAlertTime.text = when {
                dateStr.isNotBlank() && timeStr.isNotBlank() -> "$dateStr · $timeStr"
                timeStr.isNotBlank() -> timeStr
                dateStr.isNotBlank() -> dateStr
                else -> "Alert Log"
            }

            val teacherStatus = if (item.teacherPresent) "Teacher Present" else "Teacher Absent"
            binding.tvAlertContext.text = "At time of alert: ${item.students} students • $teacherStatus"
        }
    }
}

