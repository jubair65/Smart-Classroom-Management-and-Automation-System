package com.uap.cse316.smartclassroom.ui.history

import android.view.LayoutInflater
import android.view.ViewGroup
import android.widget.TextView
import androidx.core.content.ContextCompat
import androidx.recyclerview.widget.RecyclerView
import com.uap.cse316.smartclassroom.R
import com.uap.cse316.smartclassroom.data.model.HistoryItem
import com.uap.cse316.smartclassroom.databinding.ItemHistoryBinding
import java.text.SimpleDateFormat
import java.util.Date
import java.util.Locale

class HistoryAdapter(private val historyList: MutableList<HistoryItem> = mutableListOf()) :
    RecyclerView.Adapter<HistoryAdapter.HistoryViewHolder>() {

    fun setHistory(newHistory: List<HistoryItem>) {
        historyList.clear()
        // List already contains currentLiveItem at pos 0 and past history sorted newest-first
        historyList.addAll(newHistory)
        notifyDataSetChanged()
    }

    override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): HistoryViewHolder {
        val binding = ItemHistoryBinding.inflate(LayoutInflater.from(parent.context), parent, false)
        return HistoryViewHolder(binding)
    }

    override fun onBindViewHolder(holder: HistoryViewHolder, position: Int) {
        holder.bind(historyList[position], position)
    }

    override fun getItemCount(): Int = historyList.size

    inner class HistoryViewHolder(private val binding: ItemHistoryBinding) :
        RecyclerView.ViewHolder(binding.root) {

        fun bind(item: HistoryItem, position: Int) {
            val context = binding.root.context
            val density = context.resources.displayMetrics.density

            // 1. Clearly distinguish CURRENT LIVE SNAPSHOT (pos 0) vs PREVIOUS RECORDS (pos > 0)
            if (position == 0) {
                binding.tvSnapshotTag.text = "🟢 CURRENT LIVE SNAPSHOT"
                binding.tvSnapshotTag.setTextColor(ContextCompat.getColor(context, R.color.status_green))
                binding.tvSnapshotTag.setBackgroundColor(ContextCompat.getColor(context, R.color.status_green_light))
                binding.cardHistory.strokeColor = ContextCompat.getColor(context, R.color.status_green)
                binding.cardHistory.strokeWidth = (2.5f * density).toInt()
                binding.cardHistory.cardElevation = 4f * density
            } else {
                binding.tvSnapshotTag.text = "⏱️ PREVIOUS RECORD #${position}"
                binding.tvSnapshotTag.setTextColor(ContextCompat.getColor(context, R.color.text_secondary))
                binding.tvSnapshotTag.setBackgroundColor(ContextCompat.getColor(context, R.color.divider))
                binding.cardHistory.strokeColor = ContextCompat.getColor(context, R.color.divider)
                binding.cardHistory.strokeWidth = (1f * density).toInt()
                binding.cardHistory.cardElevation = 1f * density
            }

            // 2. Uniform Date & Time Formatting
            val displayDate = if (item.date.isNotBlank()) {
                try {
                    val sdf = SimpleDateFormat("yyyy-MM-dd", Locale.getDefault())
                    val parsed = sdf.parse(item.date)
                    if (parsed != null) {
                        SimpleDateFormat("dd MMM yyyy", Locale.getDefault()).format(parsed)
                    } else {
                        item.date
                    }
                } catch (e: Exception) {
                    item.date
                }
            } else if (item.timestamp > 0) {
                SimpleDateFormat("dd MMM yyyy", Locale.getDefault()).format(Date(item.timestamp))
            } else {
                SimpleDateFormat("dd MMM yyyy", Locale.getDefault()).format(Date())
            }
            binding.tvHistDate.text = "📅 $displayDate"

            val displayTime = if (item.time.isNotBlank()) item.time else "Recorded"
            binding.tvHistTime.text = "🕒 $displayTime"

            // 3. Occupancy and Unknown intruder counters
            binding.tvHistStudents.text = "👥 Students: ${item.students}"
            binding.tvHistUnknown.text = "🚨 Unknown: ${item.unknownCount}"
            if (item.unknownCount > 0) {
                binding.tvHistUnknown.setTextColor(ContextCompat.getColor(context, R.color.status_red))
            } else {
                binding.tvHistUnknown.setTextColor(ContextCompat.getColor(context, R.color.text_secondary))
            }

            // 4. Temperature
            binding.tvHistTemp.text = String.format(Locale.US, "🌡️ %.1f°C", item.temp)

            // 5. Teacher Status
            val isTeacherPresent = item.teacher.equals("PRESENT", ignoreCase = true)
            binding.tvHistTeacher.text = "Teacher: ${item.teacher}"
            if (isTeacherPresent) {
                binding.tvHistTeacher.setTextColor(ContextCompat.getColor(context, R.color.status_green))
                binding.tvHistTeacher.setBackgroundColor(ContextCompat.getColor(context, R.color.status_green_light))
            } else {
                binding.tvHistTeacher.setTextColor(ContextCompat.getColor(context, R.color.status_red))
                binding.tvHistTeacher.setBackgroundColor(ContextCompat.getColor(context, R.color.status_red_light))
            }

            // 6. Appliance Chips
            setChipState(binding.chipFan, "Fan", item.fan == 1, R.color.fan_active)
            setChipState(binding.chipLight, "Light", item.light == 1, R.color.light_active)
            setChipState(binding.chipAc, "AC", item.ac == 1, R.color.ac_active)
            setChipState(binding.chipProj, "Proj", item.proj == 1, R.color.projector_active)
        }

        private fun setChipState(chip: TextView, label: String, isOn: Boolean, activeColorRes: Int) {
            val context = chip.context
            if (isOn) {
                chip.text = "$label: ON"
                chip.setTextColor(ContextCompat.getColor(context, R.color.on_primary))
                chip.setBackgroundColor(ContextCompat.getColor(context, activeColorRes))
            } else {
                chip.text = "$label: OFF"
                chip.setTextColor(ContextCompat.getColor(context, R.color.text_secondary))
                chip.setBackgroundColor(ContextCompat.getColor(context, R.color.divider))
            }
        }
    }
}

