package com.uap.cse316.smartclassroom.ui.students

import android.graphics.Color
import android.graphics.drawable.GradientDrawable
import android.view.LayoutInflater
import android.view.ViewGroup
import androidx.recyclerview.widget.RecyclerView
import com.uap.cse316.smartclassroom.data.model.StudentItem
import com.uap.cse316.smartclassroom.databinding.ItemStudentBinding

class StudentAdapter(private val studentsList: MutableList<StudentItem> = mutableListOf()) :
    RecyclerView.Adapter<StudentAdapter.StudentViewHolder>() {

    fun setStudents(newStudents: List<StudentItem>) {
        studentsList.clear()
        studentsList.addAll(newStudents)
        notifyDataSetChanged()
    }

    override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): StudentViewHolder {
        val binding = ItemStudentBinding.inflate(LayoutInflater.from(parent.context), parent, false)
        return StudentViewHolder(binding)
    }

    override fun onBindViewHolder(holder: StudentViewHolder, position: Int) {
        holder.bind(studentsList[position], position)
    }

    override fun getItemCount(): Int = studentsList.size

    inner class StudentViewHolder(private val binding: ItemStudentBinding) :
        RecyclerView.ViewHolder(binding.root) {

        fun bind(item: StudentItem, position: Int) {
            val displayName = if (item.name.isNotBlank()) item.name else "Student"
            binding.tvStudentName.text = displayName

            // Avatar Initial & dynamic curated color
            val initial = displayName.trim().firstOrNull()?.uppercaseChar()?.toString() ?: "S"
            binding.tvAvatarInitial.text = initial

            val avatarColors = intArrayOf(
                Color.parseColor("#2563EB"), // Royal Blue
                Color.parseColor("#059669"), // Emerald Green
                Color.parseColor("#7C3AED"), // Deep Purple
                Color.parseColor("#0891B2"), // Cyan
                Color.parseColor("#D97706")  // Amber
            )
            val colorIndex = Math.abs(displayName.hashCode()) % avatarColors.size
            val shapeDrawable = GradientDrawable().apply {
                shape = GradientDrawable.OVAL
                setColor(avatarColors[colorIndex])
            }
            binding.avatarContainer.background = shapeDrawable

            // Rank in descending order (index 0 is the most recently entered)
            val rankText = when (position) {
                0 -> "#1 (Latest)"
                else -> "#${position + 1}"
            }
            binding.tvRankBadge.text = rankText

            // Verification & Active Status
            binding.tvStudentStatus.text = "🟢 Verified RFID • Active in Class"

            // Entering Time
            val timeDisplay = if (item.enterTime.isNotBlank()) {
                if (item.date.isNotBlank()) {
                    "🕒 Entered: ${item.enterTime} · ${item.date}"
                } else {
                    "🕒 Entered: ${item.enterTime}"
                }
            } else {
                "🕒 Entered recently"
            }
            binding.tvEnterTime.text = timeDisplay
        }
    }
}
